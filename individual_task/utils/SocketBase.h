#pragma once
#include "includes.h"

class SocketBase
{
public:
	SocketBase();
	~SocketBase();
	char* readAll();
	void sendAll(const char* buffer);
	void sendAll(string buffer);
protected:
	int sockfd;	
	void throwException(string errorMsg);
};