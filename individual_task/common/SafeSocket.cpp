#include "SafeSocket.h"

#pragma comment (lib, "ws2_32.lib")
#include <winsock2.h>
#include <stdio.h>
#include <cstdint>
#include <WS2tcpip.h>

SafeSocket::SafeSocket(SOCKET socket, int readTimeoutInMilliseconds) : _socket(socket) {
	//usingTimeout = true;
	//timeval timeout;
	//timeout.tv_sec = 5;
	//timeout.tv_usec = 0;
	//// On timeout read returns -1
	//setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
}

SafeSocket::SafeSocket(const char* address, const char* port) {
	WSADATA wsaData;
	addrinfo* result = NULL;
	addrinfo* ptr = NULL;
	addrinfo hints;
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		throw std::exception("WSAStartup failed");
	}
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(address, port, &hints, &result);
	if (iResult != 0) {
		throw std::exception("getaddrinfo failed");
	}

	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		_socket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

		if (_socket == INVALID_SOCKET) {
			throw std::exception("socket failed");
		}

		iResult = connect(_socket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(_socket);
			_socket = INVALID_SOCKET;
			continue;
		}

		break;
	}

	freeaddrinfo(result);

	if (_socket == INVALID_SOCKET) {
		throw std::exception("Unable to connect to server");
	}
}

SafeSocket::SafeSocket(int port) {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		throw std::exception("Error on create host socket");
	}

	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0) {
		WSACleanup();
		throw std::exception("Error on create host socket");
	}

	sockaddr_in serv_addr;
	uint16_t portno;

	portno = port;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		close();
		WSACleanup();
		throw std::exception("Error on create host socket");
	}


}

SafeSocket* SafeSocket::acceptClient(int readTimeoutInMilliseconds) {
	SOCKET clientSocket;
	unsigned int clilen;
	sockaddr_in cli_addr;
	listen(_socket, 5);
	clilen = sizeof(cli_addr);

	clientSocket = accept(_socket, (struct sockaddr *) &cli_addr, (int*)&clilen);
	
	if (clientSocket < 0) {
		throw std::exception("Error on accept client");
	}

	return new SafeSocket(clientSocket, readTimeoutInMilliseconds);
}

char* SafeSocket::readData(const std::function<bool()> continueCondition) {
	char buffer[1024] = { '\0' };
	int totalBytesCount = 0;

	totalBytesCount = recv(_socket, buffer, sizeof(buffer) - 1, 0);

	if (!continueCondition()) {
		return nullptr;
	}

	if (totalBytesCount <= 0) {
		throw std::exception("Error on read");
	}

	int messageBytesLength = MAXINT;

	while (messageBytesLength > totalBytesCount) {
		if (totalBytesCount >= 4) {
			messageBytesLength = 
				((int)buffer[0] << 24) +
				((int)buffer[1] << 16) +
				((int)buffer[2] << 8) +
				buffer[3];

			if (messageBytesLength <= totalBytesCount) {
				break;
			}
		}

		if (!continueCondition()) {
			return nullptr;
		}

		int additionalBytesCount = recv(_socket, buffer + totalBytesCount, sizeof(buffer) - 1 - totalBytesCount, 0);

		if (additionalBytesCount <= 0) {
			throw std::exception("Error on read");
		}

		totalBytesCount += additionalBytesCount;
	}

	return _strdup(buffer + 4);
}

void SafeSocket::sendData(const char* message) {
	char buffer[1024] = { '\0' };
	sprintf_s(buffer + 4, sizeof(buffer) - 4, message);
	int messageBytesLength = strlen(message);
	buffer[0] = (messageBytesLength >> 24) & 0xFF;
	buffer[1] = (messageBytesLength >> 16) & 0xFF;
	buffer[2] = (messageBytesLength >> 8)  & 0xFF;
	buffer[3] =  messageBytesLength		   & 0xFF;

	if (send(_socket, buffer, messageBytesLength + 4, 0) < 0) {
		throw std::exception("Error on send");
	}
}

void SafeSocket::close() {
	shutdown(_socket, SD_BOTH);
	closesocket(_socket);
}

SafeSocket::~SafeSocket() {
	close();
}
