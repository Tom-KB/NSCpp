/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */


#include "DebugChannel.h"

/**
 * DebugChannel implementation
 * 
 * This Channel can be used to test the server.
 * It will display on the stream every Conn/Disconn and message to the server and will echo any message he received to the client which send it.
 */

using namespace std;

/**
 * Constructor of the DebugChannel,
 * The ostream is the one used to display the information, could be any ostream.
 * @param stream
 */
DebugChannel::DebugChannel(const string& key, ostream& stream, bool echo, const string& logKey, bool doLogging) : logKey(logKey), stream(stream), echo(echo), logging(doLogging), Channel(key) {

}

void DebugChannel::run(stop_token st) {
	stringstream ss;
	GroupManager& groupManager = GroupManager::getInstance();
	while (!st.stop_requested()) {
		if (available()) {
			ss.str("");
			ClientData clientData = pull();

			if (clientData.type == Connection) {
				ss << "Connection of " << clientData.data[0] << endl;
			}
			else if (clientData.type == Disconnection) {
				ss << "Disconnection of " << clientData.data[0] << endl;
			}
			else if (clientData.type == DataReceived) {
				switch (clientData.connType) {
					case ConnType::TCP:
						ss << "[TCP] ";
						break;
					case ConnType::UDP:
						ss << "[UDP] ";
						break;
				}
				ss << "Messages";
				if (clientData.connType == ConnType::TCP) ss << " of " << clientData.client->getID();
				ss << " :" << endl;
				for (int i = 0; i < clientData.data.size(); ++i) {
					ss << "    | " << clientData.data[i] << endl;
				}

				if (logging) groupManager.send(logKey, "[DEBUG] " + ss.str());

				if (echo) {
					switch (clientData.connType) {
						case ConnType::TCP:
							clientData.client->send(ss.str(), ConnType::TCP);
							if (logging) groupManager.send(logKey, "[DEBUG] [TCP] ECHO : " + ss.str());
							break;
						case ConnType::UDP:
							sendUDP(&clientData.socket, &clientData.addr, clientData.ipType, ss.str());
							if (logging) groupManager.send(logKey, "[DEBUG] [UDP] ECHO : " + ss.str());
							break;
					}
				}	
			}

			stream << ss.str();
		}
	}
}