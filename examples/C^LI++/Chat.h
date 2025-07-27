#pragma once

#include <sstream>
#include <iostream>
#include <deque>
#include <shared_mutex>

#include <conio.h>
#include "Serializer.h"
#include "ClientPP.h"

class Chat {
public:
	/**
	 * @brief Constructor of the Chat class.
	 * @details This class wrap the client-side of C^LI++.
	 */
	Chat(std::shared_ptr<ClientPP> client, int maxMsg = 10);

	/**
	 * @brief Method to be called in a loop, handle the client's entries, messages, etc...
	 * @details Return false if the user typed !quit, true otherwise
	 */
	bool run();

protected:
	/**
	 * Pointer to the client's class.
	 */
	std::shared_ptr<ClientPP> client;

	/**
	 * Client's input buffer.
	 */
	std::string buffer;

	/**
	 * Double ended queue to store the messages.
	 */
	std::deque<std::string> messages;

	/**
	 * Max number of messages shown at the same time on the CLI.
	 */
	int maxMsg;

	/**
	 * Name of the client on the message channel.
	 */
	std::string name;

	/**
	 * Validity of the client's name for the message channel.
	 */
	bool nameValidity;

	/**
	 * Variable which tells if the server responded to a message.
	 */
	bool answered;

	/**
	 * Tells if the conversation have been chosen.
	 */
	bool convChosen;

	/**
	 * Name of the chosen conversation.
	 */
	std::string convName;

	/**
	 * Variable which hold the number of clients in the chosen conversation.
	 */
	std::string clientNumber;

	/**
	 * Mutex to handle the writing and reading over nameValidity and answered.
	 */
	std::shared_mutex mtx;

	/**
	 * Vector which will store the available conversation channels.
	 * The syntax here is based on channels with the "msg" prefix.
	 */
	std::vector<std::string> conversations;

	/**
	 * @brief GiveName
	 */
	void nameGivingCB(std::vector<std::string> message);

	/**
	 * @brief Draw the chat.
	 */
	void drawChat();

	/**
	 * @brief Draw the lobby of conversation.
	 */
	void drawLobby(int pos);

	/**
	 * @brief Add a message to the deque, limited by maxMsg.
	 */
	void addMessage(std::vector<std::string> message);

	/**
	 * @brief Update the number of clients in this conversation.
	 */
	void updateNbClients(std::vector<std::string> message);

	/**
	 * @brief Make the UDP authentification.
	 */
	void authentificationUDP(std::vector<std::string> message);

	/**
	 * @brief Get the conversation channels on the server
	 */
	void getConversations(std::vector<std::string> message);

	/**
	 * @brief Stop until the server answered or throw an error if it timeout.
	 */
	void waitForAnswer();


};

