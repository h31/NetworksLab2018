#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <io.h>
#include <mutex>
#include <thread>

class Server {
private:
	int port;
	int maxClientsCount;
public:
	Server(int port, int maxClientsCount);
};