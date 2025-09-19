/**
 * Project NSCpp
 * @author Thomas K/BIDI
 * @version 2.0
 */

#include "ServerPP.h"

using namespace std;

ServerPP::ServerPP(string address, int port, string separator, NSC_IP_Type ipType, bool useCiphering, const string& publicKeyFile, const string& privateKeyFile) : serverTCP{}, serverUDP{}, address(address), port(port), separator(separator), ipType(ipType), useCiphering(useCiphering) {
    Serializer& serializer = Serializer::getInstance();
    serializer.setSeparator(separator);
    GroupManager& groupManager = GroupManager::getInstance();
    groupManager.subscribe("Kick", createCallback(this, &ServerPP::kicked), "");

    // We check that the keys files exists
    if (useCiphering) {
        if (filesystem::exists(publicKeyFile) && filesystem::is_regular_file(publicKeyFile) &&
            filesystem::exists(privateKeyFile) && filesystem::is_regular_file(privateKeyFile)) {
            kxInterface = make_unique<KeyExchangeSodium>(publicKeyFile, privateKeyFile, KE_SIDE::Server);
        }
        else {
            kxInterface = make_unique<KeyExchangeSodium>(KE_SIDE::Server, true);
        }
    }
}

ServerPP::~ServerPP() {
    // Properly erase everything inside the structures of the Server
    // And wait for the channels to join()
    stopSources.clear();
    channels.clear();
    clients.clear();
    kxInterface.release();
;}

void ServerPP::run(stop_token st) {
    NSC_ConnType connType = NSC_ConnType::UDP;
    while (!st.stop_requested()) {
        ServerEventsList* eventsTCP = serverListen(serverTCP);
        ServerEventsList* eventsUDP = serverListen(serverUDP);
        for (auto events : { eventsTCP, eventsUDP }) {
            connType = connType == NSC_ConnType::UDP ? NSC_ConnType::TCP : NSC_ConnType::UDP; // Inverter
            for (int i = 0; i < events->numEvents; ++i) {
                ServerEvent& event = events->events[i];
                process(event, connType);
                free(event.data);
            }
            free(events->events);
            free(events);
        }
    }
}

void ServerPP::start() {
    // Automatic startup
    #if defined (_WIN32)
        startup();
    #endif
    serverTCP = createServer(address.data(), port, NSC_ConnType::TCP, ipType);
    serverUDP = createServer(address.data(), port, NSC_ConnType::UDP, ipType);

    if (serverTCP == NULL) {
        throw runtime_error("Can't create the server TCP");
    }
    if (serverUDP == NULL) {
        throw runtime_error("Can't create the server UDP");
    }
    
    // Start the run main loop
    runThread = jthread(
        [this](stop_token st) {
            try { this->run(st); }
            catch (const exception& e) {
                cerr << "Server run() raise exception : " << e.what() << endl;
            }
        }, stopTag.get_token());
}

void ServerPP::stop() {
    int nClients = serverTCP->numClients;
    for (int i = nClients - 1; i >= 0; --i) {
        clientDisconnect(serverTCP, i); // Disconnect each client starting from the last connected
    }

    stopTag.request_stop();
    runThread.join();

    // Properly close the servers' sockets
    closeServer(serverTCP);
    closeServer(serverUDP);

    // Request the stop of every thread
    for (const auto& it : stopSources) {
        stop_source ss = it.second;
        ss.request_stop();
    }

    // Automatic cleanup at the end of run
    #if defined (_WIN32)
        cleanup();
    #endif
}

void ServerPP::plug(shared_ptr<Channel> channel, ChannelType type) {
    const string& key = channel->getKey();
    channels.emplace(key, make_pair(type, channel));

    // Start the newly plugged channel
    { // In its own closure so i don't have to change the var name for channel
        auto it = channels.find(key);
        const shared_ptr<Channel>& channel = it->second.second;
        stopSources.emplace(channel->getKey(), stop_source{});
        
        // Give the information of the newly plugged channel
        GroupManager& groupManager = GroupManager::getInstance();
        groupManager.send("ServerChannelPlug", key);

        stop_token token = stopSources.at(channel->getKey()).get_token();

        // Start the channel and show an error message if it can't
        jthread th(
            [&channel](stop_token st) {
                try { channel->run(st); }
                catch (const exception& e) {
                    cerr << "Channel with key : " << channel->getKey() << " raise exception : " << e.what() << endl;
                }
            }, token);
        channelsThreads.push_back(move(th));
    }
}

void ServerPP::unplug(const string& key) {
    auto it = channels.find(key);
    if (it != channels.end()) {
        // Give the information of the unplugged channel
        GroupManager& groupManager = GroupManager::getInstance();
        groupManager.send("ServerChannelUnplug", key);

        // Stop the unplugged channel and erase it
        auto itSS = stopSources.find(key);
        itSS->second.request_stop();
        stopSources.erase(itSS);
        channels.erase(it);
    }
}

vector<string> ServerPP::getChannelKeys() {
    vector<string> keys;
    for (const auto& kv : channels) {
        keys.push_back(kv.first);
    }
    return keys;
}

void ServerPP::setChannelState(const std::string& key, const ChannelState& state) {
    auto it = channels.find(key);
    it->second.second->setState(state);
}

void ServerPP::kicked(const std::string& clientID) {
    auto it = clients.find(clientID);
    if (it != clients.end()) {
        Client cl = it->second->getClientStruct();
        stringstream ss;
        char ip[INET6_ADDRSTRLEN];
        switch (serverTCP->ipType) {
        case IPv4:
            ss << inet_ntop(AF_INET, &cl.sin.in.sin_addr, ip, INET6_ADDRSTRLEN) << ":" << ntohs(cl.sin.in6.sin6_port);
            break;
        case IPv6:
            ss << inet_ntop(AF_INET6, &cl.sin.in6.sin6_addr, ip, INET6_ADDRSTRLEN) << ":" << ntohs(cl.sin.in6.sin6_port);
            break;
        }

        shutdown(cl.socket, SHUT_RDWR);
        for (int i = 0; i < serverTCP->numClients; ++i) {
            if (serverTCP->clients[i].socket == cl.socket) {
                clientDisconnect(serverTCP, i);
                break;
            }
        }

        ClientData clientData{ NSC_EventType::Disconnection, NSC_ConnType::TCP, static_cast<NSC_IP_Type>(serverTCP->ipType), NULL, clientData.client->getClientStruct().sin, it->second}; // Default ClientData
        clientData.data.push_back(ss.str());
            
        // Distribution
        for (const auto& it : channels) {
            const shared_ptr<Channel>& channel = it.second.second;
            const ChannelType& type = it.second.first;

            if (hasType(type, ChannelType::METADATA)) {
                channel->push(clientData);
            }
        }
        clients.erase(it);
    }
}

void ServerPP::process(ServerEvent& event, const NSC_ConnType& connType) {
    stringstream ss;
    
    // Used to select which type of channels are concerned by the distribution
    string key{};
    ClientData clientData = { static_cast<NSC_EventType>(event.type), connType, static_cast<NSC_IP_Type>(event.ipType), event.socket, event.sin, nullptr }; // Default ClientData
    
    // Gather the clients information
    char ip[INET6_ADDRSTRLEN];
    switch (event.ipType) {
    case IPv4:
        ss << inet_ntop(AF_INET, &event.sin.in.sin_addr, ip, INET6_ADDRSTRLEN) << ":" << ntohs(event.sin.in.sin_port);
        break;
    case IPv6:
        ss << inet_ntop(AF_INET6, &event.sin.in6.sin6_addr, ip, INET6_ADDRSTRLEN) << ":" << ntohs(event.sin.in6.sin6_port);
        break;
    }

    if (event.type == NSC_EventType::Connection) {
        // Handle the client connection
        Client clientStruct{};
        clientStruct.socket = event.socket;
        clientStruct.connType = connType;
        clientStruct.ipType = event.ipType;
        clientStruct.sin = event.sin;
        shared_ptr<ClientPP> client = make_shared<ClientPP>(clientStruct);
        clients.emplace(ss.str(), move(client));

        auto it = clients.find(ss.str());
        if (it == clients.end()) return; // Invalid client
        clientData.client = it->second;
        if (clientData.client == nullptr) return; // Invalid client
        clientData.data.push_back(ss.str());

        // Distribution
        for (const auto& it : channels) {
            const shared_ptr<Channel>& channel = it.second.second;
            const ChannelType& type = it.second.first;

            if (hasType(type, ChannelType::METADATA)) {
                channel->push(clientData);
            }
        }
    }
    else if (event.type == NSC_EventType::Disconnection) {
        // Remove the client
        auto it = clients.find(ss.str());
        clientData.client = it->second;
        if (it != clients.end()) clients.erase(it);

        clientData.data.push_back(ss.str());

        // Distribution
        for (const auto& it : channels) {
            const shared_ptr<Channel>& channel = it.second.second;
            const ChannelType& type = it.second.first;

            if (hasType(type, ChannelType::METADATA)) {
                channel->push(clientData);
            }
        }
    }
    else if (event.type == NSC_EventType::DataReceived) {
        // Data processing
        // This is the case where the key is used to find the channel
        if (connType == NSC_ConnType::TCP) {
            auto it = clients.find(ss.str());
            if (it == clients.end()) return; // Invalid client
            clientData.client = it->second;
            if (clientData.client == nullptr) return; // Invalid client
        }
        else {
            clientData.client = nullptr; // UDP no client associate
        }
        Serializer& serializer = Serializer::getInstance();
        vector<string> message;
        string data(event.data, event.dataSize);
        printf("%s\n", data.c_str());

        if (connType == NSC_ConnType::TCP && !clientData.client->getHaveSecret() && useCiphering) {
            // This messaged is supposed to be the client's public key
            message = serializer.split(data);
            if (message.size() > 1) {
                key = message[0];
                kxInterface->computeSharedSecret(message[1]);
                clientData.client->setSharedSecret(kxInterface->getSharedSecret());
            }
            return; // We do not share this message further
        }
        // In this case the client is supposed to have the shared secret, we have to decrypt everything
        if (connType == NSC_ConnType::TCP && useCiphering) {
            string decrypted;
            try {
                decrypted = clientData.client->decrypt(data);
                if (decrypted == "") throw runtime_error("Error : Invalid decryption.");
            }
            catch (exception e) {
                return;
            }
            message = serializer.split(decrypted);
        }
        else {
            message = serializer.split(data);
        }

        if (message.size()) {
            key = message[0];
        }

        clientData.data = message;

        // Distribution
        for (const auto& it : channels) {
            const shared_ptr<Channel>& channel = it.second.second;
            const ChannelType& type = it.second.first;

            if (hasType(type, ChannelType::GLOBAL)) {
                channel->push(clientData);
            }
            else if (hasType(type, ChannelType::DATA)) {
                if (channel->getKey() == key) channel->push(clientData);
            }
        }
    }
}