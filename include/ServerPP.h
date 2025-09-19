/**
 * Project NSCpp
 * @author Thomas K/BIDI
 * @version 2.0
 */

#ifndef _SERVER_H
#define _SERVER_H

#include "Channel.h"
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <iostream>
#include <KISS.h>
#include "ClientPP.h"
#include <filesystem>

 /**
  * @file ServerPP.h
  * @brief Server implementation
  * @details This class can create a server in IPv4 or IPv6 with both TCP and UDP communications.
  * @details You can plugged Channel to which the event information will be propagated according to the given type of these Channel.
  * @details Ciphering is activated by default and a key exchange will be made with the Client upon connection.
  */

class ServerPP {
public:  
    /**
     * @brief Constructor of the ServerPP
     * @warning The given Public and Private keys should be valid, and you have to give both of them at the same time.
     * @details The ServerPP generates the public.key and private.key files when they are not given.
     * @param address Address used for the server's creation.
     * @param port Port used for the server's creation.
     * @param separator Separator used during this session.
     * @param ipType IP type used by this server (IPv4 or IPv6) (default : NSC_IP_Type::IPv4)
     * @param useCiphering Tells the server if you want to use ciphering or not (default : true)
     * @param publicKeyFile If different from "", force the use of the given public key.
     * @param privateKeyFile If different from "", force the use of the given private key.
     */
    ServerPP(std::string address, int port, std::string separator, NSC_IP_Type ipType = NSC_IP_Type::IPv4, bool useCiphering = true, const std::string& publicKeyFile = "", const std::string& privateKeyFile = "");

    /**
     * @brief Destructor of the ServerPP class.
     */
    ~ServerPP();

    /**
     * @brief Create the servers (TCP and UDP) with the given parameters and start the run method.
     * @details Run method is started in a jthread.
     */
    void start();
    
    /**
     * @brief Stop the server, disconnect properly every clients and clear everything.
     * @details The run thread is join before the clearing so everything can stop properly without crashes.
     */
    void stop();
    
    /**
     * @brief Plug a new channel to the server and start it in a jthread.
     * @warning The Channel's key should be unique, otherwise a Channel with the same key as another one will replace it.
     * @details This newly plugged channel will received informations based on its type.
     * @details The Channel's types are : GLOBAL, METADATA, DATA and SLEEPING and can be combine with the | operator.
     * @details METADATA : Information about Connection and Disconnection events.
     * @details GLOBAL : Every DataReceived events are redirected to these channels.
     * @details DATA : Only DataReceived events prefixed by the channel's key are redirected to them.
     * @details Sleeping : No data is send to this channel by the server but it is started and can be used by other channels.
     * @param channel A shared_ptr to the channel to plug.
     * @param type The channel type (GLOBAL, METADATA, DATA or/and SLEEPING)
     */
    void plug(std::shared_ptr<Channel> channel, ChannelType type);
    
    /**
     * @brief Unplug a channel based on its key.
     * @details Every channel should have a specific and unique key to be identified.
     * @details The channel's thread will be stopped.
     * @param key The key of the Channel to unplug.
     */
    void unplug(const std::string& key);
    
    /**
     * @brief Return a list which contains every plugged channel's keys.
     */
    std::vector<std::string> getChannelKeys();

    /**
     * @brief Update a channel state through the server.
     * @details States could be ACTIVE or INACTIVE
     * @details An INACTIVE channels will not received any data.
     * @param key Channel's key.
     * @param state Channel's new state.
     */
    void setChannelState(const std::string& key, const ChannelState& state);

    /**
     * @brief Callback used to tell the server you kicked a client.
     * @details You can notify through the "Kick" group that you have kicked a client and this callback will removed it from the server.
     * @see Automatically used by the Client::kick method.
     * @param clientID Client's ID on the server.
     */
    void kicked(const std::string& clientID);

private: 
    /**
     * Key used for the ECDH symmetricKey exchange.
     * Both the public key and the private key are stored inside the KXInterface from KISS
     */
    std::unique_ptr<KeyExchangeInterface> kxInterface;

     /**
     * Use an unordered_map to store the channels as value based on their key.
     * We will have an average O(1) search, insertion and removing of channels.
     * 
     * From the composition of Channel
     */
    std::unordered_map<std::string, std::pair<ChannelType, std::shared_ptr<Channel>>> channels;

     /**
     * Due to the usage of jthread for the implementation
     * Store in a map the pair of Channel's key and thread's stop source.
     */
    std::unordered_map<std::string, std::stop_source> stopSources;

     /**
     * Store the clients in an unordered_map based on the IP:PORT as the key.
     * We will have an average O(1) search, insertion and removing of channels.
     * 
     * From the aggregation of Client
     */
    std::unordered_map<std::string, std::shared_ptr<ClientPP>> clients;
   
    /**
    * This parameter is used to stop the run loop
    */
    std::stop_source stopTag;

    /// Server's pointers
    Server *serverTCP, *serverUDP;

    /// Server's address
    std::string address;

    /// Server's port
    int port;

    /// Server's IP type (IPv4 or IPv6)
    NSC_IP_Type ipType;
    
    /**
     * Separator of the messages for this server. 
     */
    std::string separator;
    
    /**
     * Boolean value which tells if we use the ciphering or not for this server. 
     */
    bool useCiphering;

    /// Thread of the run method.
    std::jthread runThread;

    /// Vectors to keep the jthread of each channels.
    std::vector<std::jthread> channelsThreads;

    /**
     * @brief Process the informations from an event.
     * @details Dispatch the informations (Conn., Disconn., Data to channels
     * @param event ServerEvent& structure which holds an event's information.
     * @param connType Give the information about which type of communication is used (TCP or UDP).
     */
    void process(ServerEvent& event, const NSC_ConnType& connType);

    /**
     * @brief Main loop of the server, it listens to the events and call the process method on them.
     * @param st Stop token used to stop the thread's loop.
     */
    void run(std::stop_token st);
    
};

#endif //_SERVER_H