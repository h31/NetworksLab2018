#undef UNICODE
#pragma warning(disable : 4996)
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
void SendData(SOCKET s, char * data, int len, struct sockaddr * client_addr, int size)
{
	char * buff = (char*)calloc(len + sizeof(int), sizeof(char));

	memcpy(buff, &len, sizeof(int));
	memcpy(&buff[sizeof(int)], data, len);

	sendto(s, &buff[0], len + sizeof(int), 0, client_addr, size);

	free(buff);
}

int getData(SOCKET s, char ** buff, struct sockaddr * client_addr, int size)
{
	char buf[256];
	ZeroMemory(buf, 256);
	int nsize;
	int totalSize = 0;
	int getSize = -1;
	*buff = NULL;

	//int bsize = recvfrom(ListenSocket, &buff[0], sizeof(buff) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_size);
	while (getSize < totalSize)
	{
		if ((nsize = recvfrom(s, &buf[0], sizeof(buf) - 1, 0, client_addr, &size)) == SOCKET_ERROR)
		{
			closesocket(s);

			getSize = 0;

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
				getSize = nsize - sizeof(int);

				memcpy(&totalSize, &buf[0], sizeof(int));

				*buff = (char*)calloc(totalSize, sizeof(char));

				memcpy(*buff, &buf[sizeof(int)], getSize);
			}
			else
			{
				memcpy(*buff[getSize], &buf[sizeof(int)], nsize);
				getSize += nsize;
			}
		}
	}

	return getSize;
}



int main(void)
{
	char buff[1024];

	printf("UDP DEMO echo-Server\n");

	// шаг 1 - подключение библиотеки 
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
	{
		printf("WSAStartup error: %d\n",
			WSAGetLastError());
		return -1;
	}

	// шаг 2 - создание сокета
	SOCKET my_sock;
	my_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (my_sock == INVALID_SOCKET)
	{
		printf("Socket() error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// шаг 3 - связывание сокета с локальным адресом 
	struct sockaddr_in local_addr;
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = htons(DEFAULT_PORT);

	if (bind(my_sock, (struct sockaddr *)&local_addr,
		sizeof(local_addr)))
	{
		printf("bind error: %d\n", WSAGetLastError());
		closesocket(my_sock);
		WSACleanup();
		return -1;
	}


	while (1)
	{


		struct sockaddr_in client_addr;

		char * bufData = NULL;
		int client_addr_size = sizeof(client_addr);


		//int bsize = recvfrom(ListenSocket, &buff[0], sizeof(buff) - 1, 0, (struct sockaddr *)&client_addr, &client_addr_size);
		int bsize = getData(my_sock, &bufData, (struct sockaddr *)&client_addr, client_addr_size);
		if (bsize == SOCKET_ERROR)
			printf("recvfrom() error: %d\n", WSAGetLastError());

		bufData[bsize] = 0;

		// Вывод на экран 
		printf("C=>S:%s\n", &bufData[0]);

		//sendto(ListenSocket, &buff[0], bsize, 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

		SendData(my_sock, &bufData[0], bsize, (struct sockaddr *)&client_addr, client_addr_size);
		free(bufData);
	}


	closesocket(my_sock);
	WSACleanup();

	return 0;
}