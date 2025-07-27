/**
 * @file Channel.h 
 * Project NSCpp
 * @author Thomas K/BIDI
 * @version 2.0
 */

#ifndef _CHANNEL_H
#define _CHANNEL_H

#include <NSC.h>
#include "ClientPP.h"
#include <queue>
#include <shared_mutex>

/// Channel's state, tells if the channel can receive data.
enum class ChannelState { Active, Inactive };

/// Can be used with | operator to combine them
enum class ChannelType { 
    /// Sleeping Channel (can't receive any message)
    SLEEPING = 0, 
    /// Receive the message send by a client
    DATA = 1 << 0,
    /// Receive METADATA on the communication (Conn., Disconn.)
    METADATA = 1 << 1, 
    /// Bypass the key system and get every message send to the server
    GLOBAL = 1 << 2 
};

/// Used to combine types
inline ChannelType operator|(ChannelType a, ChannelType b) {
    return static_cast<ChannelType>(static_cast<int>(a) | static_cast<int>(b));
}

/// Let you verify if a Channel have a given type easily
inline bool hasType(ChannelType channelType, ChannelType flag) {
    return (static_cast<int>(channelType) & static_cast<int>(flag)) != 0;
}

class Channel {
public: 
    /**
     * @brief Constructor of the Channel's class. 
     * @details Channel is an abstract class, you need to implement its run method in its specialization.
     * @param key Channel's key.
     */
    Channel(const std::string& key);
    
    /**
     * @brief Defined the Channel's key.
     * @warning It will not update the key on the server. You should unplug the channel, change its key and then re-plugged it for the new key to be effective.
     * @param key New Channel's key.
     */
    void setKey(const std::string& key);
    
    /**
     * @brief Return the Channel's key.
     */
    const std::string& getKey();
    
    /**
     * @brief Change the state of the server.
     * @details States are ChannelState::Active or ChannelState::Inactive
     * @details When inactive a channel will not received any data even if it is triggered by a client.
     * @param state New state to apply.
     */
    void setState(ChannelState state);
    
    /**
     * @brief Add the given data at the end of the queue.
     * @details Handles automatically the lock.
     * @param data ClientData structure holding the data from the event.
     */
    void push(ClientData data);
    
    /**
     * @brief Pop the first data from the queue and return it.
     * @details Handles automatically the lock.
     */
    ClientData pull();

    /**
     * @brief Return true if data is available inside the dataQueue, false otherwise.
     */
    bool available();
    
    /**
     * @brief This method is run in a Thread and handle the Channel.
     * @param Stop token used to stop the run loop.
     */
    virtual void run(std::stop_token st) = 0;

protected: 
    /**
     * This key must be unique on the server on which it is plugged.
     */
    std::string key;

    /**
     * This queue is used to store (and read) the data send to the Channel by the Server
     */
    std::queue<ClientData> dataQueue;

    /**
     * Control if the channel is active or not.
     */
    ChannelState state;

private: 
    std::shared_mutex mtx; // .lock_shared() for read-only access | .lock() to write access
};

#endif //_CHANNEL_H