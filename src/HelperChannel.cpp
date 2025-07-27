/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */


#include "HelperChannel.h"

/**
 * HelperChannel implementation
 * 
 * This Channel return the list of current active channels on the server, can be use to synced with the Client (cliend-sided) class to provide a real-time handling of the channels states.
 */

using namespace std;

HelperChannel::HelperChannel(const string& key, const string& logKey, bool doLogging) : logKey(logKey), logging(doLogging), Channel(key) {
	// In this conception we supposed the used of a key to directly questionned the channel
	GroupManager& groupManager = GroupManager::getInstance();
	groupManager.subscribe("ServerChannelPlug", createCallback(this, &HelperChannel::addChannelKey), this->getKey());
	groupManager.subscribe("ServerChannelUnplug", createCallback(this, &HelperChannel::removeChannelKey), this->getKey());
}

void HelperChannel::run(stop_token st) {
	GroupManager& groupManager = GroupManager::getInstance();
	Serializer& serializer = Serializer::getInstance();
	while (!st.stop_requested()) {
		if (available()) {
			serializer.clear(key);
			ClientData clientData = pull();
			if (clientData.connType != ConnType::TCP) continue; // Skip UDP

			serializer.add(key, key);
			if (clientData.type == DataReceived) {
				for (const auto& it : keys) {
					serializer.add(key, it);
				}

				if (logging) groupManager.send(logKey, "[HELPER] List of channels' keys send to " + clientData.client->getID() + " : " + serializer.get(key) + "\n");
				clientData.client->send(serializer.get(key));
			}
		}
	}
}

void HelperChannel::addChannelKey(const string& key) {
	keys.insert(key);
}

void HelperChannel::removeChannelKey(const string& key) {
	keys.erase(key);
}

