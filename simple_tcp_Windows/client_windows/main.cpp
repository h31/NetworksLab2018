#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>

#include <WinSock2.h>
#include <string.h>
#include <thread>

#pragma comment(lib, "Ws2_32.lib")
#pragma pack(push, 1)

#define BUFSIZE 65535

typedef struct Data_s{
	char dataSize;
	char data[256];
} Data;

#pragma pack(pop)

char buffer[BUFSIZE];

int main(int argc, char *argv[]) {
    SOCKET sockfd;
    short portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

	WSADATA WSStartData;

	if (WSAStartup(MAKEWORD(2, 2), &WSStartData) != 0) {
		perror("ERROR on WSAStartup");
		exit(1);
	}

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (int) atoi(argv[2]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == INVALID_SOCKET) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
		closesocket(sockfd);
		shutdown(sockfd, 2);
        exit(0);
    }

    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memmove(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        perror("ERROR connecting");
		closesocket(sockfd);
		shutdown(sockfd, 2);
        exit(1);
    }

    printf("Please enter the message: ");
    memset(buffer, 0, BUFSIZE + 1);
    fgets(buffer, BUFSIZE, stdin);

    size_t size = strlen(buffer);
    size_t iter = 0;
    Data data;
    int rem; 

    while((rem = (int)size - iter) > 0){
		if (rem > 255) data.dataSize = 255;
		else data.dataSize = rem;
		memcpy(data.data, buffer, data.dataSize);
		int result = send(sockfd, (char*)&data, (int)data.dataSize + 1, NULL);
		if (result < 0) {
			perror("ERROR connection lost");
			closesocket(sockfd);
			shutdown(sockfd, 2);
			exit(2);
		}
		iter += data.dataSize;
		Sleep(0);
    }

    data.dataSize = 0;
    send(sockfd, (char*)&data, 1, NULL);
	closesocket(sockfd);
	shutdown(sockfd, 2);
    return 0;
}
