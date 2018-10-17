#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma warning(disable:4996)

DWORD WINAPI listen_func(LPVOID p);

DWORD WINAPI communicate_func(LPVOID p);

int main(int argc, char *argv[]) {
	SOCKET s;
	WSADATA wsa;
	unsigned int portno;
	struct sockaddr_in serv_addr;


	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		return 1;
	}
	printf("Initialised.\n");

	/* First call to socket() function */
	s = socket(AF_INET, SOCK_STREAM, 0);

	if (s == INVALID_SOCKET) {
		perror("ERROR opening socket");
		exit(1);
	}

	/* Initialize socket structure */
	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	portno = 5001;

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	/* Now bind the host address using bind() call.*/
	if (bind(s, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR on binding");
		closesocket(s);
		exit(1);
	}

	/* Now start listening for the clients, here process will
	* go in sleep mode and will wait for the incoming connection */
	listen(s, 5);

	//thread for listening
	HANDLE listen_thread = CreateThread(NULL, 0, listen_func, &s, 0, NULL);

	if (listen_thread == NULL) {
		perror("Error while creating listen thread");
		closesocket(s);
		exit(1);
	}

	/* if pressed q then exit program */
	int key = 0;
	while (1) {
		key = getchar();
		if (key == 'q') break;
	}
	closesocket(s);
	_getch();
	exit(0);
}

DWORD WINAPI listen_func(LPVOID p) {
	SOCKET *s = ((SOCKET *)p)[0];
	int c;
	SOCKET new_socket;
	struct sockaddr_in cli_addr;

	c = sizeof(struct sockaddr_in);
	while (1) {

		new_socket = accept(s, (struct sockaddr *)&cli_addr, &c);
		if (new_socket == INVALID_SOCKET)
		{
			perror("ERROR on accept");
			printf("code: %d\n", WSAGetLastError());
			ExitThread(1);
		}

		//print connection
		HOSTENT *hst;
		hst = gethostbyaddr((char *)&cli_addr.sin_addr.s_addr, 4, AF_INET);
		printf("+%s [%s] new connect!\n", (hst) ? hst->h_name : "", inet_ntoa(cli_addr.sin_addr));

		/* Making new thread for messaging with client */
		HANDLE communicate_thread = CreateThread(NULL, 0, communicate_func, &new_socket, 0, NULL);

		if (communicate_thread == NULL) {
			perror("Error while creating thread for client");
			closesocket(new_socket);
		}
	}
	printf("exit from listen_func\n");
	ExitThread(0);
}

DWORD WINAPI communicate_func(LPVOID p) {
	SOCKET *s = ((SOCKET *)p)[0];

	char buffer[256];
	char output[300]; //put message in buffer 
	int n;
	int message_length = 0;
	int received_length = 0;
	int read_length = 0;

	memset(output, 0, 300);
	
	strcat(output, "Here is the message: ");
	
	/* If connection is established then start communicating
	First we need to read length of message */
	n = recv(s, (char*)&message_length, sizeof(int), 0); 

	if (n < 0) {
		perror("ERROR reading length of the message from socket");
		closesocket(s);
		ExitThread(1);
	}
	
	while (received_length <= message_length) {
		memset(buffer, 0, 256);
		read_length = message_length - received_length;
		if (read_length > 255) read_length = 255;
		n = recv(s, buffer, read_length, 0);
		if (n < 0) {
			printf("ERROR reading from socket");
			closesocket(s);
			ExitThread(1);
		}
		received_length += n;
		strcat(output, buffer);
	}
	printf("%s\n", output);

	n = send(s, "I got your message", 18, 0);
	
	if (n < 0) {
		perror("ERROR writing to socket");
		closesocket(s);
		ExitThread(1);
	}
	
	closesocket(s); //close socket after messaging
	ExitThread(0);
}
