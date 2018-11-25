#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORTION 9
#define DEFAULT_CLIENT_COUNT 4
#define DEFAULT_PORT "27015"

struct shared_data
{
	HANDLE mutex;
	HANDLE client_threads[DEFAULT_CLIENT_COUNT];
	int client_count;
	SOCKET socket_array[DEFAULT_CLIENT_COUNT];
	

	shared_data()
	{
		client_count = 0;
		mutex = NULL;
		for (int i = 0; i < DEFAULT_CLIENT_COUNT; ++i)
		{
			client_threads[i] = NULL;
			socket_array[i] = INVALID_SOCKET;
		}
	}
};

shared_data g_shared_data;

void kill_client(int n)
{
	WaitForSingleObject(g_shared_data.mutex, INFINITE);
	auto sock = g_shared_data.socket_array[n];
	if (sock == INVALID_SOCKET ||
		sock == SOCKET_ERROR) return;
	printf("shutting down client socket #%d\n", n);
	int iResult = shutdown(sock, SD_BOTH);
	if (iResult == SOCKET_ERROR) {
		printf("socket #%d still in use\n", n);
	}
	closesocket(sock);
	g_shared_data.socket_array[n] = INVALID_SOCKET;
	ReleaseMutex(g_shared_data.mutex);
}

bool startsWith(const char* str, const char* starts)
{
	if (str == 0 || starts == 0) return false;
	int length = strlen(starts);
	if (strlen(str) < length) return false;
	for (int i = 0; i < length; ++i)
	{
		if (str[i] != starts[i]) return false;
	}
	return true;
}

int readn(SOCKET sock, char* buf, int buflen, int _n)
{
	ZeroMemory(buf, buflen);
	int iResult;
	int n = 0;
	while (n < _n)
	{
		iResult = recv(sock, buf + n, _n - n, 0);
		if (iResult > 0) {
			n += iResult;
		}
		else if (iResult == 0)
		{
			printf("connection terminated by remote user...\n");
			return SOCKET_ERROR;
		}
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			return SOCKET_ERROR;
		}
	}
	return n;
}


DWORD WINAPI client_thread(void* data)
{
	int number = *((int*)data);

	int iResult, iSendResult;
	char recvbuf[DEFAULT_BUFLEN] = { 0 };
	int recvbuflen = DEFAULT_BUFLEN;

	// Accept a client socket
	SOCKET sock = g_shared_data.socket_array[number];
	
	// No longer need server socket
	printf("%d: connected to remote user\n", number);
	printf("%d: receiving data...\n", number);
	// Receive until the peer shuts down the connection
	do {
		ZeroMemory(recvbuf, sizeof(recvbuf));

		iResult = readn(sock, recvbuf, DEFAULT_BUFLEN, DEFAULT_PORTION);
		if (iResult > 0) {
			printf("%d: Received:'%s' (%d bytes)\n", number, recvbuf, iResult);

			// Echo the buffer back to the sender
			iSendResult = send(sock, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("%d: send back failed with error: %d\nEXITING\n", number, WSAGetLastError());

				break;
			}
			printf("%d: Sending back:'%s' (%d bytes)\n", number, recvbuf, iSendResult);
		}
		else if (iResult == SOCKET_ERROR)
		{
			break;
		}

	} while (iResult > 0);


	if (sock != INVALID_SOCKET)
	{
		kill_client(number);
	}


	delete ((int*)data);

	printf("%d: disconnected\n", number);
	return 0;
}

DWORD WINAPI listen_thread(void* data)
{
	SOCKET server_sock = *((SOCKET*)data);

	// Accept a client socket
	int number = g_shared_data.client_count;
	while (true)
	{
		SOCKET& sock = g_shared_data.socket_array[number];

		sock = accept(server_sock, NULL, NULL);

		if (sock == INVALID_SOCKET) {
			printf("server socket failure, disconnecting clients\n");
			break;
		}

		if (number < DEFAULT_CLIENT_COUNT)
		{
			int* params = new int;
			*params = number;
			WaitForSingleObject(g_shared_data.mutex, INFINITE);
			g_shared_data.client_threads[number] = CreateThread(NULL, 0, client_thread, params, 0, NULL);
			ReleaseMutex(g_shared_data.mutex);
			++number;
		}
		else
		{
			printf("warning: Could not connect new client: out of slots.\n");
		}
	}

	for (int i = 0; i < DEFAULT_CLIENT_COUNT; ++i)
	{
		kill_client(i);
	}

	printf("waiting for client threads to finish...\n");

	WaitForMultipleObjects(DEFAULT_CLIENT_COUNT, g_shared_data.client_threads, TRUE, INFINITE);

	delete ((SOCKET*)data);

	return 0;
}

int main(void)
{
	HANDLE lthread;
	WSADATA wsaData;

	g_shared_data.mutex = CreateMutex(NULL, FALSE, NULL);
	if (g_shared_data.mutex == NULL)
	{
		printf("failed to create mutex\n");
		return 1;
	}

	struct addrinfo *result = NULL;
	struct addrinfo hints;
	int iResult;

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
	SOCKET server_sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (server_sock == INVALID_SOCKET) {
		printf("server socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	printf("listening for port %s...\n", DEFAULT_PORT);
	iResult = bind(server_sock, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(server_sock);
		server_sock = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(server_sock, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(server_sock);
		server_sock = INVALID_SOCKET;
		WSACleanup();
		return 1;
	}

	SOCKET* params = new SOCKET;
	*params = server_sock;
	lthread = CreateThread(NULL, 0, listen_thread, params, 0, NULL);

	char cmdbuf[DEFAULT_BUFLEN] = { 0 };

	while (true)
	{
		gets_s(cmdbuf, DEFAULT_BUFLEN);

		if (strcmp(cmdbuf, "exit") == 0)
		{
			break;
		}
		else if (strlen(cmdbuf) == 0)
		{
			;
		}
		else if (strcmp(cmdbuf, "list") == 0)
		{
			printf("connection list:\n");
			WaitForSingleObject(g_shared_data.mutex, INFINITE);
			for (int i = 0; i < DEFAULT_CLIENT_COUNT; ++i)
			{
				auto sock = g_shared_data.socket_array[i];
				if (sock != INVALID_SOCKET &&
					sock != SOCKET_ERROR)
				{
					printf("%d: remote user connected on socket %d\n", i, sock);
				}
			}
			ReleaseMutex(g_shared_data.mutex);
		}
		else if (startsWith(cmdbuf, "kill"))
		{
			char num[16] = { 0 };
			strcpy_s(num, 15, cmdbuf + 4);
			if (strlen(num) == 0) return false;
			int n = atoi(num);
			kill_client(n);
		}
		else
		{
			printf("Unrecognized command. Use 'exit' to exit, 'kill 0..%d' to disconnect client, 'list' to list all clients\n", DEFAULT_CLIENT_COUNT - 1);
		}
	}


	printf("shutting down server socket...\n");

	// shutdown the connection since we're done
	iResult = shutdown(server_sock, SD_BOTH);
	if (iResult == SOCKET_ERROR) {
		printf("server socket still in use\n");
	}
	closesocket(server_sock);

	WaitForSingleObject(lthread, INFINITE);

	CloseHandle(g_shared_data.mutex);

	WSACleanup();

	printf("main thread finished\n");

	getchar();
	return 0;
}
