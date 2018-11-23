#pragma once
#include "Client.h"
#include "DB.h"

class Server : public SocketBase {
public:
    explicit Server(uint16_t port);
	~Server();
	static void deleteClient(Client* client);
	static DB* db;
private:
	const int backlog = 5;
	static vector<Client*> clients;
	void mainBody();
	void closeAndExit(string errorMsg, int errorCode);
};