#include "ThreadSafeStoreWrapper.h"
#include "DirectoryTreeStore.h"
#include "ThreadSafeLinkedList.h"
#include "SafeSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <thread>

class Server {
private:
	const int port;
	const int maxClientsCount;
	bool closed = false;
	SafeSocket serverSocket;
	ThreadSafeLinkedList<SafeSocket*> clientsList;
	Store* store = new ThreadSafeStoreWrapper(new DirectoryTreeStore());

	void closeServer();
	void acceptNewClients();
	void handleClientConnection(SafeSocket* clientSocket);
	void cleanUpClientConnection(SafeSocket* clientSocket);

public:
	Server(int port, int maxClientsCount);
	~Server();
	void start();
};