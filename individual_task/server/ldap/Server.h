#include "ThreadSafeStoreWrapper.h"

#pragma comment (lib, "ws2_32.lib")
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <io.h>
#include <mutex>
#include <thread>

class Server {
private:
	const int port;
	const int maxClientsCount;
	const int readTimeoutInMilliseconds;
	int activeClientsCount = 0;
	std::mutex activeClientsCountMutex;
	bool closed = false;
	SOCKET serverSocket = 0;

	void closeAllClients();
	void acceptNewClients();
	void handleClientConnection(SOCKET newsockfd);
	void closeSocket(SOCKET socket);
	void cleanUpClientConnectionThread(SOCKET socket);

public:
	Server(int port, int maxClientsCount, int readTimeoutInMilliseconds);
	void start();
};