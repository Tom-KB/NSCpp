/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */


#include "Channel.h"

using namespace std;

Channel::Channel(const string& key) : key(key), state(ChannelState::Active) {

}

/**
 * @param key
 */
void Channel::setKey(const string& k) {
	key = k;
}

const string& Channel::getKey() {
	return key;
}

/**
 * Change the state of the server.
 * ChannelState::Active or ChannelState::Inactive
 * When inactive a channel will not received any data even if it is triggered by a client.
 * @param state
 */
void Channel::setState(ChannelState st) {
	state = st;
}

/**
 * Add the given data at the end of the queue.
 * Handle automatically the lock.
 * @param data
 */
void Channel::push(ClientData data) {
	if (state == ChannelState::Inactive) return;
	scoped_lock lock(mtx);
	dataQueue.push(data);
}

/**
 * Pop the first data from the queue and return it.
 * Handle automatically the lock.
 */
ClientData Channel::pull() {
	scoped_lock lock(mtx); // Automatically unlock when out-of-scope
	ClientData cD{};
	if (dataQueue.size()) {
		cD = dataQueue.front();
		dataQueue.pop();
	}
	return cD;
}

bool Channel::available() {
	return dataQueue.size() > 0;
}