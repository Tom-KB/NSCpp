/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */


#include "GroupManager.h"

/**
 * GroupManager implementation
 * 
 * This class is a Singleton.
 * Manage group of communication.
 * Usage : 
 * The channels subscribe/unsubscribe to groups via :
 * - subscribe(groupName, receiver, CUID)
 * - unsubscribe(groupName, CUID)
 * 
 * They can then use the send(groupName, message) to propagate to other channel in groupName the given message.
 * When a message is send to a Channel, the given receiver functions are called.
 */

using namespace std;

GroupManager& GroupManager::getInstance() {
    static GroupManager instance;
    return instance;
}

/**
 * @param groupName
 * @param receiver
 * @param CUID
 */
void GroupManager::subscribe(const std::string& groupName, function<void(const string&)> callback, const std::string& CUID) {
    groups[groupName].push_back({callback, CUID});
}

/**
 * @param groupName 
 * @param CUID
 */
void GroupManager::unsubscribe(const std::string& groupName, const std::string& CUID) {
   auto it = groups.find(groupName);
   if (it != groups.end()) {
       erase_if(it->second, [&](const auto& p) {
           return p.second == CUID;
       });
    }
}

/**
 * @param groupName
 * @param message
 */
void GroupManager::send(const std::string& groupName, const std::string& message) {
    auto it = groups.find(groupName);
    if (it != groups.end()) {
        for (const auto& it2 : it->second) {
            if (it2.first) it2.first(message);
        }
    }
}