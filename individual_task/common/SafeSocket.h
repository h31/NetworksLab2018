#pragma comment (lib, "ws2_32.lib")
#include <functional>
#include <winsock2.h>

class SafeSocket {
private:
	SOCKET _socket;
	SafeSocket(SOCKET socket, int readTimeoutInMilliseconds);
	bool usingTimeout = false;
public:
	SafeSocket(int port);
	SafeSocket(const char* address, const char* port);
	SafeSocket* acceptClient(int readTimeoutInMilliseconds);
	char* readData(const std::function<bool()> continueCondition);
	void sendData(const char* message);
	void close();
	~SafeSocket();
};