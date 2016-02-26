#pragma once
#include "stdafx.h"

class User {
public:

	User() {
		socket = new sf::TcpSocket;
		socket->setBlocking(true);
	}
	~User() {
		delete socket;
	}
	void startPingTimer() {
		pingTimer = new sf::Clock();
		pingTimer->restart();
	}
	void stopPingTimer() {
		delete pingTimer;
	}

	std::string username;
	sf::TcpSocket* socket;
	sf::Clock* pingTimer;

};
