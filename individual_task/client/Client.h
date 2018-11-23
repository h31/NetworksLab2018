#pragma once
#include "SocketBase.h"

class Client : public SocketBase {
public:
	Client(char* hostname, char* port);
	~Client();
	void handler();
private:
	struct sockaddr_in serv_addr;
	void closeAndExit(string errorMsg, int errorCode);
	bool validate(char* str);
	bool doLogin();
};