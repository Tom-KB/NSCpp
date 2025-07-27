#pragma once
#include <Channel.h>
#include <DebugChannel.h>
#include <sstream>

class MessageChannel : public Channel {
public:
    /**
     * @brief Constructor of the DebugChannel.
     * @details The ostream is the one used to display the information, could be any ostream.
     * @param key Channel's key.
     * @param logKey Key of the log channel group which can be used to log the debug information (default : "logChannel").
     * @param doLogging Tells if the Channel should log its actions.
     */
    MessageChannel(const std::string& key, const std::string& logKey = "logChannel", bool doLogging = true);

    /**
     * @brief Run method of the Channel, process the given events.
     */
    void run(std::stop_token st);

protected:
    /**
     * Tells if the channel is supposed to log.
     */
    bool logging;

    /**
     * Key of the log channel's group.
     */
    std::string logKey;

    /**
     * Map the clients names to their ClientPP structure.
     */
    std::unordered_map<std::string, std::shared_ptr<ClientPP>> clients;

    /**
     * Map the clients names to their UDP address.
     */
    std::unordered_map<std::string, SIN> clientsAddrUDP;

    /**
     * Const reference to the server's socket.
     */
    SOCKET serverSocket;

    /**
     * Used IP_Type.
     */
    IP_Type usedIpType;

    /**
     * Tells if the IP_Type is valid. (Have been gathered from a received message)
     * We implement that for the sake of the configurable example.
     */
    bool validIpType;

    /**
     * @brief Add the pair client's name - clientPtr to the map.
     * @details Return true if the client name is free, false otherwise.
     */
    bool addClient(std::string name, std::shared_ptr<ClientPP> client);

    /**
     * @brief Return a user name based on the client. O(n) here.
     */
    std::string getName(std::shared_ptr<ClientPP> client);

    /**
     * @brief Ask the client about its UDP authentification or tells it that its valid.
     */
    void UDPAuth(std::shared_ptr<ClientPP> client, std::string validity);
};

