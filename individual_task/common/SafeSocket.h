#pragma comment (lib, "ws2_32.lib")
#include <winsock2.h>

class SafeSocket {
private:
	SOCKET _socket;
	SafeSocket(SOCKET socket);
public:
	SafeSocket(int port);
	SafeSocket(const char* address, const char* port);
	SafeSocket* acceptClient();
	char* readData();
	void sendData(const char* message);
	void close();
	~SafeSocket();
};