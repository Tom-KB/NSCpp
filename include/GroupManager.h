/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */


#ifndef _GROUPMANAGER_H
#define _GROUPMANAGER_H

#include <functional>
#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <vector>
#include <string>
#include <sstream>

 /// Template which can be used to create group callbacks from class' methods.
template <typename T>
auto createCallback(T* ptr, void (T::* method)(const std::string&)) {
    return [ptr, method](const std::string& s) {
        (ptr->*method)(s);
        };
}

class GroupManager {
public: 
    /**
     * @brief Return the instance of the GroupManager or create one if it does not exist.
     * @details This class is a Singleton.
     */
    static GroupManager& getInstance();
    
    /**
     * @brief Let you register a Channel's callback to a group.
     * @param groupName Group's name.
     * @param callback Callback which will be called when someone emit in the groupName's group.
     * @param CUID Channel's key used to identify it.
     */
    void subscribe(const std::string& groupName, std::function<void(const std::string&)> callback, const std::string& CUID);
    
    /**
     * @brief Remove a Channel's callback from the given group.
     * @param groupName Group's name to search for the callback.
     * @param CUID Channel's key in the group associate to the callback.
     */
    void unsubscribe(const std::string& groupName , const std::string& CUID);
    
    /**
     * @brief Emit the message in the given groupName.
     * @details Every callbacks of the given group will be called with the message as parameter.
     * @param groupName Group's name.
     * @param message Message to be emit the group.
     */
    void send(const std::string& groupName, const std::string& message);

protected: 
    /**
     * Unordered map of the groups, link the group's name to the callback and CUID.
     * Multiple callbacks can be declared for the same group.
     * When a Channel is unsubscribe, every receiver inside the group is removed.
     */
    std::unordered_map<std::string, std::vector<std::pair<std::function<void(const std::string&)>, std::string>>> groups;

private: 
    
    GroupManager() = default;
    ~GroupManager() = default;

    // Design to be used by multiple threads =>
    std::shared_mutex mtx;

    // We block the different way to copy or deplace it
    GroupManager(const GroupManager&) = delete;
    GroupManager& operator=(const GroupManager&) = delete;
    GroupManager(GroupManager&&) = delete;
    GroupManager& operator=(GroupManager&&) = delete;
};

#endif //_GROUPMANAGER_H