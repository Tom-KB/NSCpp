/**
 * Project NSCpp
 * @author TK
 * @version 2.0
 */


#include "LogChannel.h"

using namespace std;

/**
 * LogChannel implementation
 * 
 * Will add on the fstream the log of the server (Message received, Conn, Disconn) in a formatted way.
 * The format includes the date, the type of element and the IP address  related to the event.
 */


/**
 * @param stream
 */
LogChannel::LogChannel(const string& key, const std::string& logFilePrefix) : prefix(logFilePrefix), Channel(key) {
	today = chrono::system_clock::now();
	string todayStr = format("{:%Y-%m-%d}", today);
	getStream(prefix + todayStr + ".log");

	GroupManager& groupManager = GroupManager::getInstance();
	groupManager.subscribe(key, createCallback(this, &LogChannel::log), this->getKey());
}

void LogChannel::run(stop_token st) {
	stringstream ss;

	while (!st.stop_requested()) {
		if (available()) {
			ss.str("");
			ClientData clientData = pull();

			loadTime(ss);

			if (clientData.type == Connection) {
				ss << " [CONNECTION] " << clientData.data[0] << endl;
			}
			else if (clientData.type == Disconnection) {
				ss << " [DISCONNECTION] " << clientData.data[0] << endl;
			}
			else if (clientData.type == DataReceived) {
				ss << " [DATA] ";
				if (clientData.connType == ConnType::TCP) ss << clientData.client->getID() << " ";
				ss << ":" << endl;

				for (int i = 0; i < clientData.data.size(); ++i) {
					ss << "                            | " << clientData.data[i] << endl;
				}
			}

			stream << ss.str();
			stream.flush();
		}
	}
	stream.close();
}

void LogChannel::log(const std::string& logEntry) {
	scoped_lock lock(mtx);
	stringstream ss;
	loadTime(ss);
	ss << " ";
	ss << logEntry;
	stream << ss.str();
	if (!ss.str().ends_with("\n")) stream << endl;
	stream.flush();
}

void LogChannel::getStream(const std::string& name) {
	stream.open(name, std::ios::out | std::ios::app);
	if (!stream.is_open()) {
		throw std::runtime_error("Error : could not open " + name);
	}
}

void LogChannel::loadTime(std::stringstream& ss) {
	auto now = chrono::system_clock::now();

	// Verification of the date for the logging
	if (floor<chrono::days>(today) != floor<chrono::days>(today)) {
		// Change file for each new day
		today = now;
		string todayStr = format("{:%Y-%m-%d}", today);
		getStream(prefix + todayStr + ".log");
	}

	chrono::zoned_time local_time{ chrono::current_zone(), floor<chrono::seconds>(now) };
	string nowStr = format("{:%Y-%m-%d %H:%M:%S}", local_time);

	ss << nowStr;
}


