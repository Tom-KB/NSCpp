/**
 * Project NSCpp
 * @author Thomas K/BIDI
 * @version 2.0
 */

#include "ClientPP.h"

using namespace std;

ClientPP::ClientPP(Client cInfo) : clientUDP{}, type(type), isDisconnected(false), haveSecret(false), useCiphering(true) {
    clientTCP = (Client*)malloc(sizeof(Client)); // In the server case, it's just a structure with some elemental information
    if (clientTCP) {
        clientTCP->socket = cInfo.socket;
        clientTCP->connType = cInfo.connType;
        clientTCP->ipType = cInfo.ipType;
        ipType = static_cast<NSC_IP_Type>(clientTCP->ipType);
        clientTCP->sin = cInfo.sin;
        init();
    }
    else {
        throw runtime_error("Error : Couldn't create the client.");
    }
}


ClientPP::ClientPP(const string& address, int port, const string& separator, NSC_IP_Type ipType, bool useCiphering) : clientTCP{}, clientUDP{}, address(address), port(port), ipType(ipType), type(ClientType::LOCAL), isDisconnected(true), haveSecret(false), useCiphering(useCiphering) {
    Serializer& serializer = Serializer::getInstance();
    serializer.setSeparator(separator);
}

ClientPP::~ClientPP() {
    dataRcvCB.clear();
    disconnCB.clear();
    kxInterface.release();
}

void ClientPP::init() {
    stringstream ss;
    char ip[INET6_ADDRSTRLEN];
    switch (clientTCP->ipType) {
    case IPv4:
        ss << inet_ntop(AF_INET, &clientTCP->sin.in.sin_addr, ip, INET6_ADDRSTRLEN) << ":" << ntohs(clientTCP->sin.in.sin_port);;
        break;
    case IPv6:
        ss << inet_ntop(AF_INET6, &clientTCP->sin.in6.sin6_addr, ip, INET6_ADDRSTRLEN) << ":" << ntohs(clientTCP->sin.in6.sin6_port);
        break;
    }
    ID = ss.str();

    switch (type) {
    case ClientType::LOCAL:
        if (!useCiphering) {
            kxInterface = nullptr;
            return;
        }
        // Check if keys have been given
        if (PKFile != "" && SKFile != "") {
            kxInterface = make_unique<KeyExchangeSodium>(PKFile, SKFile, KE_SIDE::Client);
        }
        else {
            // Create a new pair of keys and their files
            kxInterface = make_unique<KeyExchangeSodium>(KE_SIDE::Client, true);
        }
        if (ServerPKFile == "") throw runtime_error("Error : Server's public key have not been given.");

        unsigned char serverPK[crypto_kx_PUBLICKEYBYTES];
        loadBase64Key(serverPK, crypto_kx_PUBLICKEYBYTES, ServerPKFile);
        kxInterface->computeSharedSecret(toString(serverPK, crypto_kx_PUBLICKEYBYTES)); // Compute the shared secret with the server
        this->setSharedSecret(kxInterface->getSharedSecret()); // Setup the symmetric ciphering with this secret
        break;
    case ClientType::REMOTE:
        kxInterface = nullptr; // Not used in this case
        break;
    }
}

void ClientPP::send(const string& data, NSC_ConnType connType) {
    if (isDisconnected) return;
    string message = data;
    switch (clientTCP->ipType) {
    case IPv4:
        if (connType == NSC_ConnType::TCP) {
            if (useCiphering && haveSecret) message = symmetricCipher->encrypt(data);
            sendMessage(&clientTCP->socket, message.c_str(), static_cast<uint32_t>(message.size()), connType, clientTCP->ipType, &clientTCP->sin);
        }
        else if (type == ClientType::LOCAL) { // Can't use this function in REMOTE
            // In the UDP case we don't use encryption
            sendMessage(&clientUDP->socket, message.c_str(), static_cast<uint32_t>(message.size()), connType, clientTCP->ipType, &clientUDP->sin);
        }
        break;
    case IPv6:
        if (connType == NSC_ConnType::TCP) {
            if (useCiphering && haveSecret) message = symmetricCipher->encrypt(data);
            sendMessage(&clientTCP->socket, message.c_str(), static_cast<uint32_t>(message.size()), connType, clientTCP->ipType, &clientTCP->sin);
        }
        else if (type == ClientType::LOCAL) { // Can't use this function in REMOTE
            // In the UDP case we don't use encryption
            sendMessage(&clientUDP->socket, message.c_str(), static_cast<uint32_t>(message.size()), connType, clientTCP->ipType, &clientTCP->sin);
        }
        break;
    }
}

const string& ClientPP::getID() {
    return ID;
}

void ClientPP::kick() {
    if (isDisconnected || type == ClientType::LOCAL) return;
    GroupManager& groupManager = GroupManager::getInstance();
    groupManager.send("Kick", ID);
    isDisconnected = true;
}

const Client& ClientPP::getClientStruct() {
    return *clientTCP;
}

void ClientPP::start() {
    if (type != ClientType::LOCAL) {
        throw runtime_error("You can't start a remote client.");
    }

    runThread = jthread([this](stop_token st){
#if defined(_WIN32)
        startup();
#endif

        clientTCP = createClient(address.c_str(), port, NSC_ConnType::TCP, ipType);
        clientUDP = createClient(address.c_str(), port, NSC_ConnType::UDP, ipType);

        if (!clientTCP || !clientUDP) {
            cerr << "Error creating client" << endl;
            return;
        }

        isDisconnected = false;
        init();

        if (useCiphering) {
            Serializer& serializer = Serializer::getInstance();
            serializer.add("publicKey", "publicKeyPublishing");
            serializer.add("publicKey", kxInterface->getPublicKey());

            useCiphering = false;
            send(serializer.get("publicKey"));  // fait dans le thread
            useCiphering = true;
            serializer.clear("publicKey");
        }

        try { this->run(st); }
        catch (const std::exception& e) { cerr << "Client run() raise exception: " << e.what() << endl; }

    }, stopTag.get_token());
}

void ClientPP::addDataCallback(const string& key, function<void(vector<string>)> callback) {
    dataRcvCB[key] = callback;
}

void ClientPP::addDisconnectionCallback(std::function<void(void)> callback) {
    disconnCB.push_back(callback);
}

bool ClientPP::getHaveSecret() {
    return haveSecret;
}

void ClientPP::stop() {
    if (type != ClientType::LOCAL) return;
    stopTag.request_stop();
    runThread.join(); // Wait for the thread to end
    closeClient(clientTCP); // Disconnect properly the client
    closeClient(clientUDP);
    // Automatic cleanup at the end of run
#if defined (_WIN32)
    cleanup();
#endif
}

void ClientPP::setSharedSecret(const string& sharedSecret) {
    symmetricCipher = make_shared<SymmetricCipherSodium>(sharedSecret);
    haveSecret = true;
}

std::string ClientPP::decrypt(const string& cipher) {
    if (useCiphering && haveSecret) return symmetricCipher->decrypt(cipher);
    return cipher;
}

void ClientPP::setKeysPaths(const string& PK, const string& SK) {
    if (type != ClientType::LOCAL) return;
    PKFile = PK;
    SKFile = SK;
}

void ClientPP::setServerPKPath(const string& SPKFile) {
    if (type != ClientType::LOCAL) return;
    ServerPKFile = SPKFile;
}

void ClientPP::run(std::stop_token st) {
    while (!st.stop_requested()) {
        ClientEventsList* eventsTCP = clientListen(clientTCP);
        ClientEventsList* eventsUDP = clientListen(clientUDP);
        string key;
        NSC_ConnType connType = NSC_ConnType::UDP;
        for (auto events : { eventsTCP, eventsUDP }) {
            connType = connType == NSC_ConnType::UDP ? NSC_ConnType::TCP : NSC_ConnType::UDP; // Inverter
            for (int i = 0; i < events->numEvents; i++) {
                ClientEvent& event = events->events[i];
                if (event.type == NSC_EventType::Disconnection) {
                    for (const auto& it : disconnCB) {
                        if (it) it();
                    }
                    stopTag.request_stop();
                }
                else if (event.type == NSC_EventType::DataReceived) {
                    string data(event.data, event.dataSize);
                    Serializer& serializer = Serializer::getInstance();
                    vector<string> message;

                    if (connType == NSC_ConnType::TCP && useCiphering) {
                        string decrypted;
                        try {
                            decrypted = this->decrypt(data);
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

                    auto it = dataRcvCB.find(key);
                    if (it != dataRcvCB.end()) {
                        if (it->second) it->second(message); // Callback on the key
                    }
                    else {
                        if (dataRcvCB.find("") != dataRcvCB.end()) {
                            if (dataRcvCB[""]) dataRcvCB[""](message); // Default callback
                        }
                    }

                    free(event.data);
                }
            }
            free(events->events);
            free(events);
        }
    }
}

void sendUDP(SOCKET* serverSocket, SIN* addr, NSC_IP_Type ipType, string message) {
    sendMessage(serverSocket, message.c_str(), static_cast<uint32_t>(message.size()), NSC_ConnType::UDP, ipType, addr);
}

std::string domainNameResolution(std::string domainName) {
    return resolveDomainName(domainName.c_str());
}
