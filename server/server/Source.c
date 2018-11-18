#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

// Эта функция создается в отдельном потоке и
// обсуживает очередного подключившегося клиента
// независимо от остальных

void SendData(SOCKET s,char * data, int len)
{
	char * buff = (char*)calloc(len + 1, sizeof(char));

	memcpy(buff, data, len);
	buff[len] = 0;
	//memcpy(&buff[sizeof(int)], data, len);

	send(s, buff, len + 1, 0);

	free(buff);
}

int getData(SOCKET s, char ** buff)
{
	char buf[256];
	ZeroMemory(buf, 256);
	int nsize;
	int totalSize = 0;
	int getSize = 0;
	*buff = NULL;


	while (1)
	{
		if ((nsize = recv(s, &buf[0], sizeof(buf) - 1, 0)) == SOCKET_ERROR)
		{
			closesocket(s);

			getSize = -1;

			if (*buff != NULL)
			{
				free(*buff);
				*buff = NULL;
			}
		}

		if (nsize > 0)
		{
			if (totalSize == 0)
			{
				*buff = (char*)calloc(nsize, sizeof(char));
				totalSize += nsize;
			}
			else if (!(nsize + getSize < totalSize))
			{
				totalSize += nsize;
				*buff = (char *)realloc(*buff, totalSize);
			}

			memcpy(&*buff[getSize], &buf[0], nsize);
			getSize += nsize;

			if (buf[255] == 0)
			{
				break;
			}
		}
	}

	return getSize;
}


DWORD WINAPI SexToClient(LPVOID client_socket)
{
	SOCKET my_sock;
	my_sock = ((SOCKET *)client_socket)[0];
	#define sHELLO "Hello, Sailor\r\n"

	SendData(my_sock, sHELLO, sizeof(sHELLO));
	char * buff=NULL;
	int size;


	while ((size=getData(my_sock, &buff))>0)
	{
		SendData(my_sock, buff, size);
	}

	free(buff);
	closesocket(my_sock);
	return 0;
}

int __cdecl main(void)
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo *result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	SOCKET client_socket;

	while ((client_socket = accept(ListenSocket, NULL, NULL)))
	{

		// пытаемся получить имя хоста
		HOSTENT *hst;

			// Вызов нового потока для обслужвания клиента
			// Да, для этого рекомендуется использовать
			// _beginthreadex но, поскольку никаких вызов
			// функций стандартной Си библиотеки поток не
			// делает, можно обойтись и CreateThread
		DWORD thID;
		CreateThread(NULL, NULL, SexToClient, &client_socket, NULL, &thID);
	}


	closesocket(ListenSocket);
	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}