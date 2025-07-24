/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */


#ifndef _CLIENT_H
#define _CLIENT_H

#include <NSC.h>
#include "GroupManager.h"
#include "Serializer.h"
#include <string>
#include <KISS.h>
#include <memory>
#include <vector>
#include <iostream>
#include <sstream>

 /**
  * @file ClientPP.h
  * @brief Client implementation
  *
  * @details This Client class give a high level usage of the clients socket.
  * @details The objective is to facilitate the usage of the developper.
  * @details It has two ways of functioning.
  * @details The first one is when it's created by the Server, in this case it represent a Client connected to the server and can be used by the channels to communicate with the client [REMOTE].
  * @details The second case is when it's created in the client-sided application, in this case it can be used to communicate with the server [LOCAL].
  * @details The class is also made to let you the freedom of derived it for a more convenient usage.
  */

enum class ClientType {
    /// Client-Side
    LOCAL, 
    /// Server-Side
    REMOTE 
};

class ClientPP;

/// Structure which hold the Client's Data after an event.
typedef struct ClientData {
    /// Type of the event : Connection, Disconnection, DataReceived.
    EventType type;
    /// Connection type of this event : TCP or UDP.
    ConnType connType;
    /// IP type : IPv4 or IPv6.
    IP_Type ipType;
    /// In TCP -> Client's socket; in UDP -> Server's socket.
    SOCKET socket;
    /// Client's address.
    SIN addr;
    /// Shared pointer of the client which produced the event (nullptr for UDP).
    std::shared_ptr<ClientPP> client;
    /// Vector with the string obtained after a split on the separator.
    std::vector<std::string> data;
} ClientData;

/// Template which can be used to create DataReceived callbacks from class' methods.
template <typename T>
auto createDataCallback(T* ptr, void (T::* method)(std::vector<std::string>)) {
    return [ptr, method](std::vector<std::string> v) {
        (ptr->*method)(v);
        };
}

/// Template which can be used to create Disconnection callbacks from class' methods.
template <typename T>
auto createDisconnectionCallback(T* ptr, void (T::* method)()) {
    return [ptr, method]() {
        (ptr->*method)();
        };
}

/**
 * @brief Function to send an UDP message to a client.
 * @details Should be used on the server-side.
 * @see ClientPP::send To send messages from the client side
 */
void sendUDP(SOCKET* serverSocket, SIN* addr, IP_Type ipType, std::string message);

class ClientPP {
public:
    /**
     * @brief Constructor of the REMOTE client.
     * @param clientInfo Client structure with the necessary information.
     */
    ClientPP(Client clientInfo);

    /**
     * @brief Constructor of the LOCAL client.
     * @param address Server's address.
     * @param port Server's port.
     * @param separator Separator used during this session.
     * @param ipType IP type (IPv4 or IPv6) of this connection.
     * @param useCiphering Indication about the usage of ciphering in this session.
     */
    ClientPP(const std::string& address, int port, const std::string& separator, IP_Type ipType = IP_Type::IPv4, bool useCiphering = true);

    /**
     * @brief Destructor of the ClientPP class.
     */
    ~ClientPP();
    
    /**
     * @brief This method can be used by both LOCAL and REMOTE client to exchange with the Server/Clients.
     * @warning For REMOTE clients, don't use this method for UDP communications.
     * @see sendUDP to answer with UDP.
     * @param data Data to send to the peer (client or server)
     * @param connType Type of connection to be used for the message, could be ConnType::TCP or ConnType::UDP (default : ConnType::TCP)
     */
    void send(const std::string& data, ConnType connType = ConnType::TCP);
    
    /**
     * @brief Return the Client's identity string.
     * @details Which is "address:port".
     * @warning Is an ID only locally, client-side or server-side, not accross the network.
     */
    const std::string& getID();

    /**
     * @brief Kick the client by closing its socket.
     */
    void kick();

    /**
     * @brief Return a reference to the client structure.
     */
    const Client& getClientStruct();

    /**
     * @brief Connect the client and start the run loop.
     * @details Method for the LOCAL client which handles the connection and start the run method in a jthread.
     */
    void start();

    /**
     * @brief Add a callback for the DataReceived event.
     * @details Each time a message with the given key as prefix is received, the callback will be called with the received data as parameter.
     * @details Key is unique, but you can create a callback which will propagate the information.
     * @param key Prefix which will trigger the callback method. Should be unique.
     * @param callback Callback of the following form void(vector<string>).
     */
    void addDataCallback(const std::string& key, std::function<void(std::vector<std::string>)> callback);

    /**
     * @brief Add a callback for the Disconnection event.
     * @details Called when a disconnection happen.
     * @param callback Callback of the following form void(void).
     */
    void addDisconnectionCallback(std::function<void(void)> callback);

    /**
     * @brief Return the haveSecret boolean value.
     * @details haveSecret is true when the shared secret has been compute.
     * @details It does not tell you that the secret is the same between the server and the client.
     */
    bool getHaveSecret();

    /**
     * @brief Stop the client by terminating its run loop and disconnect it from the server.
     * @warning Only available on LOCAL clients.
     * @details Also, properly clear clientTCP and clientUDP.
     */
    void stop();

    /**
     * @brief Let you set the public key to compute the shared secret.
     * @details The given string will be considered as the peer's public key and use to compute the shared secret.
     */
    void setSharedSecret(const std::string& PK);

    /**
     * @brief Return the decrypted string of the given cipher.
     * @param cipher Cipher to decrypt.
     */
    std::string decrypt(const std::string& cipher);

    /**
     * @brief Set the public and private keys for the [LOCAL] client.
     * @warning Only available for the LOCAL client.
     * @param PKFile Path to the public key's file.
     * @param SKFile Path to the secret key's file.
     */
    void setKeysPaths(const string& PKFile, const string& SKFile);

    /**
     * @brief Set the server's public key for the [LOCAL] client.
     * @warning Only available for the LOCAL client.
     * @param SPKFile Path to the server's public key file
     */
    void setServerPKPath(const string& SPKFile);

protected: 
    /**
     * Have the information of the symmetricKey used for the communication between the Server and the Client.
     * Server-side : keep the information for the server.
     */
    std::shared_ptr<SymmetricCipherInterface> symmetricCipher;

    /**
     * Key used for the ECDH symmetricKey exchange.
     * Both the public key and the private key are stored inside the KXInterface from KISS.
     */
    std::unique_ptr<KeyExchangeInterface> kxInterface;
     
    /// TCP client pointer
    Client* clientTCP;

    /// UDP client pointer
    Client* clientUDP;

    /**
     * Information of the client's type REMOTE or LOCAL.
     */
    ClientType type;

    /**
     * String "address:port"
     */
    std::string ID;

    /**
     * Boolean which is used to stop using the client when kicked
     */
    bool isDisconnected;

    /**
    * This parameter is used to stop the run loop
    */
    std::stop_source stopTag;

    /**
     * @brief Main loop of the client, it listens to the events and process them.
     * @param st Stop token used to stop the loop of the thread.
     */
    void run(std::stop_token st);

    /**
     * @brief Init the different informations of the client.
     * @details Its ID and KISS classes when needed.
     */
    void init();

    /**
     * Tells if the secret has been computed.
     */
    bool haveSecret;

    /**
     * Tells if the client use ciphering.
     */
    bool useCiphering;

    /// Run thread
    std::jthread runThread;
    
    /// Server's address
    std::string address;
    /// Server's port
    int port;
    /// Session's IP type (IPv4 or IPv6)
    IP_Type ipType;

    /**
     * This map register the callback for the DataReceived's events.
     * "" is the default callback, called when no key is detected, if it does not exist the message is just ignored.
     */
    std::unordered_map<std::string, std::function<void(std::vector<std::string>)>> dataRcvCB;

    /**
     * This map register the callback for the Disconnection's events.
     */
    std::vector<std::function<void(void)>> disconnCB;

    /**
     * Paths to the public and private keys.
     */
    std::string PKFile, SKFile;

    /**
     * Path to the server Public Key file.
     */
    std::string ServerPKFile;
};

#endif //_CLIENT_H