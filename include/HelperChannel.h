/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */


#ifndef _HELPERCHANNEL_H
#define _HELPERCHANNEL_H

#include "Channel.h"
#include "GroupManager.h"
#include <sstream>
#include <iostream>
#include <unordered_set>

class HelperChannel: public Channel {
public:
    /**
     * @brief Constructor of the HelperChannel.
     * @param key Channel's key.
     * @param logKey Key of the log channel group which can be used to log the debug information (default : "logChannel").
     * @param doLogging Tells if the Channel should log its actions.
     */
    HelperChannel(const std::string& key, const std::string& logKey = "logChannel", bool doLogging = true);
    
    /**
     * @brief Run method of the Channel, process the given events.
     */
    void run(std::stop_token st);

    /**
     * @brief Used as a callback which listen to the "ServerChannelPlug" group.
     * @details When a new Channel is plug, the server emit in a group which call this method to add the newly plugged Channel.
     * @param key Newly plugged Channel's key.
     */
    void addChannelKey(const std::string& key);

    /**
     * @brief Used as a callback which listen to the "ServerChannelUnplug" group.
     * @details When a Channel is unplug, the server emit in a group which call this method to remove the unplugged Channel.
     * @param key Unplugged Channel's key.
     */
    void removeChannelKey(const std::string& key);

protected:
    /**
     * Keep the keys of the plugged channels.
     */
    std::unordered_set<std::string> keys;

    /**
     * Tells if the channel is supposed to log.
     */
    bool logging;

    /**
     * Key of the log channel's group.
     */
    std::string logKey;
};

#endif //_HELPERCHANNEL_H