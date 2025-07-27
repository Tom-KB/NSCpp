#include <iostream>
#include <NSCpp.h>
#include <deque>

#include "Chat.h"

// Channels includes
#include "MessageChannel.h"

using namespace std;

int main() {
	// Configuration
		// For this example, you must left TCP in the configuration. (UDP is tested by the nbClients exchange)
	ConnType usedConnType = ConnType::TCP; 
	IP_Type usedIpType = IP_Type::IPv4;
	const char* address = usedIpType == IP_Type::IPv4 ? "127.0.0.1" : "::1";

	try {
		string launchType;
		cout << "Choose between this two options (e.g. : \"1\") : \n" << "    1 - Server\n    2 - Client" << endl;
		getline(cin, launchType);

		if (launchType == "1") {
			// Server
			unique_ptr<ServerPP> server = make_unique<ServerPP>(address, 25565, "/", usedIpType, true, "keys/public.key", "keys/private.key");
			shared_ptr<Channel> debugChannel = make_shared<DebugChannel>("debugChannel", cout, false, "logs");
			shared_ptr<LogChannel> logChannel = make_shared<LogChannel>("logs", "logs/log-");
			shared_ptr<HelperChannel> helperChannel = make_shared<HelperChannel>("discovery", "logs");


			server->plug(debugChannel, ChannelType::GLOBAL | ChannelType::METADATA);
			server->plug(helperChannel, ChannelType::DATA);
			server->plug(logChannel, ChannelType::SLEEPING);
			// You could also 
			server->plug(make_shared<MessageChannel>("msgGaming", "logs"), ChannelType::DATA | ChannelType::METADATA);
			server->plug(make_shared<MessageChannel>("msgBoxing", "logs"), ChannelType::DATA | ChannelType::METADATA);
			server->plug(make_shared<MessageChannel>("msgDevelopment", "logs"), ChannelType::DATA | ChannelType::METADATA);
			server->plug(make_shared<MessageChannel>("msgBooks", "logs"), ChannelType::DATA | ChannelType::METADATA);
			server->plug(make_shared<MessageChannel>("msgManga", "logs"), ChannelType::DATA | ChannelType::METADATA);
			server->plug(make_shared<MessageChannel>("msgLOTR", "logs"), ChannelType::DATA | ChannelType::METADATA);

			// To showcase the helper channel's system I add a lobby where you can choose your channel.
			// The syntax of the key must be "msg{TOPIC}" e.g. : "msgGaming" will appeared as "Gaming" in the lobby.

			server->start();

			// Use the ! key to close the server
			char c = '0';
			while (true) {
				if (_kbhit()) {
					c = _getch();
					if (c == '!') break;
				}
			}

			server->stop();
		}
		else if (launchType == "2") {
			// Client
			shared_ptr<ClientPP> client = make_shared<ClientPP>(address, 25565, "/", usedIpType, true);
			client->setServerPKPath("serverPK.key");
			client->start();
			//Sleep(1000);
			unique_ptr<Chat> chat = make_unique<Chat>(client);
			while (chat->run()) {}

			client->stop();
		}
		else {
			printf("You had to choose between 1 and 2.");
			Sleep(1000);
		}
	}
	catch (const exception& e) {
		printf("%s\n", e.what());
	}
}
