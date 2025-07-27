/**
 * Project NSCpp
 * @author Thomas K/BIDI
 * @version 2.0
 */


#ifndef _DEBUGCHANNEL_H
#define _DEBUGCHANNEL_H

#include "Channel.h"
#include "GroupManager.h"
#include <sstream>
#include <iostream>

class DebugChannel: public Channel {
public: 
    /**
     * @brief Constructor of the DebugChannel.
     * @details The ostream is the one used to display the information, could be any ostream.
     * @param key Channel's key.
     * @param stream Stream on which the debug will be shown.
     * @param echo Tells if the Channel echo back the data to the clients.
     * @param logKey Key of the log channel group which can be used to log the debug information (default : "logChannel").
     * @param doLogging Tells if the Channel should log its actions.
     */
    DebugChannel(const std::string& key, std::ostream& stream, bool echo = false, const std::string& logKey = "logChannel", bool doLogging = true);
    
    /**
     * @brief Run method of the Channel, process the given events.
     */
    void run(std::stop_token st);

protected:
    /**
     * Debug's stream.
     */
    std::ostream& stream;

    /**
     * Tells if the channel is supposed to log.
     */
    bool logging;

    /**
     * Tells if the channel echo back (with the same protocol) to the client.
     */
    bool echo;

    /**
     * Key of the log channel's group.
     */
    std::string logKey;
};

#endif //_DEBUGCHANNEL_H