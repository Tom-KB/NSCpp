#include "Chat.h"
using namespace std;

void clearScreen() {
	system("cls");
}

Chat::Chat(shared_ptr<ClientPP> client, int maxMsg) : client(client), maxMsg(maxMsg), name(""), nameValidity(false), answered(false), convChosen(false), clientNumber("#") {
	client->addDataCallback("discovery", createDataCallback(this, &Chat::getConversations));
	client->addDataCallback("identify", createDataCallback(this, &Chat::nameGivingCB));
	client->addDataCallback("identifyUDP", createDataCallback(this, &Chat::authentificationUDP));
	client->addDataCallback("message", createDataCallback(this, &Chat::addMessage));
	client->addDataCallback("nbClients", createDataCallback(this, &Chat::updateNbClients));
	client->send("discovery");
	waitForAnswer();
}

bool Chat::run() {
	Serializer& serializer = Serializer::getInstance();

	unsigned char c = '0';
	// Lobby commands
	if (!convChosen) {
		int choice = 0;
		drawLobby(choice);
		while (1) {
			if (_kbhit()) {
				c = _getch();
				if (c == 224) {
					// Check for up and down arrow
					c = _getch();
					if (c == 72) { // Up
						if (choice > 0) choice--;
						drawLobby(choice);
					}
					else if (c == 80) { // Down
						if (choice < conversations.size() - 1) choice++;
						drawLobby(choice);
					}
				}
				if (c == '\r') {
					convName = conversations[choice];
					convChosen = true;
					break;
				}
			}
		}
	}
	try {
		waitForAnswer(); // We wait for the server answered if necessary (answered set to false)
	}
	catch (exception e) {
		answered = true;
		nameValidity = false;
	}

	{ // Block for the scoped_lock
		scoped_lock lock(mtx);
		if (!nameValidity) {
			cout << "Enter your name for this conversation : " << endl;
			getline(cin, buffer);
			serializer.edit("chat", "msg" + convName, 0);
			serializer.edit("chat", "identify", 1);
			serializer.edit("chat", buffer, 2); // Given name
			name = buffer;
			answered = false;
			client->send(serializer.get("chat"));

			serializer.edit("chat", "message", 1);
			return true;
		}
	}

	// Send the UDP auth., even if the packet did'nt arrive don't worry, the server will
	// automatically tell the client to resend it.
	serializer.edit("identifyUDP", "msg" + convName, 0);
	serializer.edit("identifyUDP", "identifyUDP", 1);
	serializer.edit("identifyUDP", name, 2);
	client->send(serializer.get("identifyUDP"), ConnType::UDP);
	serializer.clear("identifyUDP");

	buffer.clear();
	drawChat();
	while (1) {
		if (_kbhit()) {
			c = _getch();
			if (c == '\r') break;
			if (c == '\b') { // Backspace
				if (buffer.size() <= 0) continue;
				buffer.pop_back();
				drawChat();
			}
			else {
				buffer.push_back(c);
				printf("%c", c);
			}
		}
	}
	if (buffer == "!quit") {
		return false;
	}
	if (!buffer.size()) return true;

	// Add the message locally and emit it to the server
	serializer.edit("chat", buffer, 2);

	addMessage({"", "You : " + buffer});

	client->send(serializer.get("chat"));

	return true;
}

void Chat::nameGivingCB(vector<string> message) {
	scoped_lock lock(mtx);
	answered = true;
	if (message.size()) {
		if (message[1] == "valid") {
			nameValidity = true;
			return; // Name is good
		}
		// "invalid" case
		cout << "Name already taken. Change it please." << endl;
	}
}

void Chat::drawChat() {
	clearScreen();
	printf("------------------------ C^LI++ ------------------------\n");
	printf("%s - %s\n", convName.c_str(), clientNumber.c_str());
	for (int i = 0; i < maxMsg; ++i) {
		if (i < messages.size()) printf("%s\n\n", messages[i].c_str());
		else printf("\n\n");
	}
	printf("Enter a message : %s", buffer.c_str());
}

void Chat::drawLobby(int pos) {
	clearScreen();
	printf("------------------------ C^LI++ ------------------------\n");
	cout << "------------------------ TOPIC -------------------------" << endl;
	for (int i = 0; i < conversations.size(); ++i) {
		if (i == pos) cout << "> ";
		cout << conversations[i] << endl;
	}
}

void Chat::addMessage(vector<string> message) {
	scoped_lock lock(mtx);
	if (messages.size() < maxMsg) {
		messages.push_back(message[1]);
	}
	else {
		messages.pop_front();
		messages.push_back(message[1]);
	}
	drawChat();
}

void Chat::updateNbClients(std::vector<std::string> message) {
	scoped_lock lock(mtx);
	if (message.size()) {
		clientNumber = message[1];
		drawChat();
	}
}

void Chat::authentificationUDP(std::vector<std::string> message) {
	if (message.size()) {
		if (message[1] == "valid") {
			return;
		}
	}
	// If invalid, or not receive we resend the message.
	Serializer& serializer = Serializer::getInstance();
	client->send(serializer.get("identifyUDP"), ConnType::UDP);
}

void Chat::getConversations(std::vector<std::string> message) {
	scoped_lock lock(mtx);
	for (int i = 0; i < message.size(); ++i) {
		if (message[i].starts_with("msg")) {
			conversations.push_back(message[i].substr(3));
		}
	}
	answered = true;
}

void Chat::waitForAnswer() {
	if (!answered) {
		int i = 0;
		while (true) {
			Sleep(500);
			{
				scoped_lock lock(mtx);
				if (answered) break;
			}
			i++;
			if (i >= 20) throw runtime_error("Error : answer timeout.");
		}
	}
}
