#define WIN32_LEAN_AND_MEAN
#pragma warning(disable : 4996)

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define SERVERADDR "127.0.0.1"

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




int main(int argc, char *argv[])
{

	char buff[10 * 1014];
	printf("UDP DEMO Client\nType quit to quit\n");

	// Шаг 1 - иницилизация библиотеки Winsocks
	if (WSAStartup(0x202, (WSADATA *)&buff[0]))
	{
		printf("WSAStartup error: %d\n",
			WSAGetLastError());
		return -1;
	}

	// Шаг 2 - открытие сокета
	SOCKET my_sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (my_sock == INVALID_SOCKET)
	{
		printf("socket() error: %d\n", WSAGetLastError());
		WSACleanup();
		return -1;
	}

	// Шаг 3 - обмен сообщений с сервером
	HOSTENT *hst;
	struct sockaddr_in dest_addr;

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(DEFAULT_PORT);

	// определение IP-адреса узла
	if (inet_addr(SERVERADDR))
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else
		if (hst = gethostbyname(SERVERADDR))
			dest_addr.sin_addr.s_addr = ((unsigned long **)
				hst->h_addr_list)[0][0];
		else
		{
			printf("Unknown host: %d\n", WSAGetLastError());
			closesocket(my_sock);
			WSACleanup();
			return -1;
		}

	while (1)
	{
		// чтение сообщения с клавиатуры
		printf("S<=C:"); fgets(&buff[0], sizeof(buff) - 1,
			stdin);
		if (!strcmp(&buff[0], "quit\n")) break;

		// Передача сообщений на сервер

		SendData(my_sock, &buff[0], strlen(&buff[0]), (struct sockaddr *)&dest_addr, sizeof(dest_addr));

		// Прием сообщения с сервера
		char * bufData = NULL;
		struct sockaddr_in server_addr;
		int server_addr_size = sizeof(server_addr);

		int bsize = getData(my_sock, &bufData, (struct sockaddr *)&server_addr, server_addr_size);

		if (bsize == SOCKET_ERROR)
		{
			printf("recvfrom() error:"\
				"%d\n", WSAGetLastError());
			closesocket(my_sock);
			WSACleanup();
			return -1;
		}

		buff[bsize] = 0;

		// Вывод принятого с сервера сообщения на экран
		printf("S=>C:%s", &buff[0]);
	}

	// шаг последний - выход
	closesocket(my_sock);
	WSACleanup();

	return 0;
}
