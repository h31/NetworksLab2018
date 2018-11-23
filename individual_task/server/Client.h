#pragma once
#include "SocketBase.h"

class Client : public SocketBase {
public:
	Client();
	~Client();
	int acceptConnection(int sockfd);
	void handler();
	bool operator==(const Client& client);
private:
	struct sockaddr_in cli_addr;
	string name;
	const char* id;
	bool isClient;
	void handleAdmin();
	void handleClient();
    int validateMonthAndYear(string monthStr, string yearStr);
};