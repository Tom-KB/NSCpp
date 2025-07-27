#include "MessageChannel.h"

using namespace std;

MessageChannel::MessageChannel(const string& key, const string& logKey, bool doLogging) : logKey(logKey), logging(doLogging), serverSocket(NULL), Channel(key) {
	usedIpType = IPv4; // Default, only used for UDP messages, automatically update with the first received message.
	validIpType = false;
}

void MessageChannel::run(stop_token st) {
	GroupManager& groupManager = GroupManager::getInstance();
	Serializer& serializer = Serializer::getInstance();

	auto start = chrono::high_resolution_clock::now();

	while (!st.stop_requested()) {
		// Every ~5s update the statistics of the conversation to every clients in UDP.
		auto end = chrono::high_resolution_clock::now();
		chrono::duration<double, std::milli> elapsed = end - start;

		if (elapsed.count() >= 5000) {
			if (serverSocket != NULL) {
				serializer.edit("answer", "nbClients", 0);
				string nbClients = to_string(clients.size());
				serializer.edit("answer", nbClients, 1);
				string message = serializer.get("answer");

				// Check every client, if they're auth. in UDP send them the number of clients.
				// And if they're not auth. in UDP, ask them for it.
				for (const auto& it : clients) {
					auto itUDP = clientsAddrUDP.find(it.first);
					if (itUDP != clientsAddrUDP.end()) {
						SIN addr = itUDP->second;
						sendUDP(&serverSocket, &addr, usedIpType, message);
					}
					else {
						UDPAuth(it.second, "invalid");
					}
				}
			}
			start = chrono::high_resolution_clock::now();
		}

		if (available()) {
			ClientData clientData = pull();

			// Add logging
			if (clientData.type == EventType::Disconnection) {
				string name = getName(clientData.client);
				if (name != "") {
					// The client is logged here, we remove it
					clients.erase(name);
					auto addrIT = clientsAddrUDP.find(name);
					if (addrIT != clientsAddrUDP.end()) clientsAddrUDP.erase(name);

					serializer.edit("answer", "message", 0);
					serializer.edit("answer", "SYSTEM : " + name + " disconnected.", 1);
					string message = serializer.get("answer");
					for (const auto& it : clients) {
						it.second->send(message);
					}
					if (logging) groupManager.send(logKey, "[CONVERSATION] " + key + " [SEND] - " + message);
				}
			}
			else if (clientData.type == EventType::DataReceived) {
				if (!validIpType) {
					usedIpType = clientData.ipType;
					validIpType = true;
				}
				
				if (clientData.connType == ConnType::UDP && validIpType) {
					if (clientData.data.size()) {
						if (serverSocket == NULL) serverSocket = clientData.socket;
						if (clientData.data[1] == "identifyUDP") {
							// UDP's address identification
							string name = clientData.data[2];

							auto it = clients.find(name); // The client must be logged in TCP.
							string validity;
							if (it != clients.end()) {
								clientsAddrUDP[name] = clientData.addr;
								validity = "valid";
							}
							else {
								validity = "invalid";
							}
							UDPAuth(it->second, validity);
						}
					}
					continue;
				}

				// Process message
				if (clientData.data.size()) {
					string msgType = clientData.data[1]; // Get the message type
					if (msgType == "identify") {
						// {key}/identify/{name}
						string name = clientData.data[2];
						string isValid = addClient(name, clientData.client) ? "valid" : "invalid";
						serializer.edit("answer", "identify", 0);
						serializer.edit("answer", isValid, 1);
						clientData.client->send(serializer.get("answer"));

						if (logging && isValid == "valid") groupManager.send(logKey, "[CONVERSATION] " + key + " [IDENTIFY] - " + clientData.client->getID() + " as " + name);
					}
					else if (msgType == "message") {
						// {key}/message/{message}
						serializer.edit("answer", "message", 0);
						string message = clientData.data[2];
						string clientName = getName(clientData.client);
						if (clientName == "") continue; // Broken case, a non identified user shouldn't send any messages

						serializer.edit("answer", clientName + " : " + message, 1);
						message = serializer.get("answer");
						for (const auto& it : clients) {
							if (it.second == clientData.client) continue;
							it.second->send(message); // message/{name} : {message}
						}

						if (logging) groupManager.send(logKey, "[CONVERSATION] " + key + " [MESSAGE] - " + clientName + " : " + message);
					}
				}
			}
		}
	}
}

bool MessageChannel::addClient(string name, shared_ptr<ClientPP> client) {
	auto it = clients.find(name);
	if (it != clients.end()) return false;
	clients[name] = client;
	return true;
}

string MessageChannel::getName(shared_ptr<ClientPP> client) {
	for (const auto& it : clients) {
		if (it.second == client) return it.first;
	}
	return "";
}

void MessageChannel::UDPAuth(shared_ptr<ClientPP> client, string validity) {
	Serializer& serializer = Serializer::getInstance();
	// We answer in TCP, to tell the client that we received its UDP authentification.
	serializer.edit("answer", "identifyUDP", 0);

	serializer.edit("answer", validity, 1);

	client->send(serializer.get("answer")); // TCP
}
