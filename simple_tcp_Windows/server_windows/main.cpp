#include <stdio.h>

#include <WinSock2.h>
#include <string.h>
#include <thread>

#pragma comment(lib, "WS2_32.lib")
#pragma pack(push, 1)

using namespace std;

typedef struct Data_s{
	char dataSize;
	char data[256];
} Data;

#pragma pack(pop)

DWORD WINAPI connection_handler(LPVOID temp);

int main(int argc, char *argv[]) {

	WSADATA WSStartData; 
	if (WSAStartup(MAKEWORD(2, 2), &WSStartData) != 0) {
		perror("ERROR on WSAStartup");
		exit(1);
	}

    SOCKET sockfd, newsockfd;
    unsigned short portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) 
    {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (::bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("ERROR on binding");
    	closesocket(sockfd);
		shutdown(sockfd, 2);
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    if(listen(sockfd, 5) < 0)
    {
		perror("listen");
		closesocket(sockfd);
		shutdown(sockfd, 2);
		exit(EXIT_FAILURE);
    }

    puts("Waiting for incoming connections");
    clilen = sizeof(cli_addr);

    while( (newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, (int *)&clilen)))
    {

		puts("Connection accepted");

		thread sn_thread(connection_handler, &newsockfd);

		if(sn_thread.joinable())
		{
			perror("Could not create thread");
			closesocket(sockfd);
			shutdown(sockfd, 2);
			return 1;
		}
		sn_thread.detach();
	
		puts("Handler assigned");
	}

	if (sockfd < 0)
	{
		perror("accept failed");
		return 1;
    }

    closesocket(sockfd);
    exit(0);
}

DWORD WINAPI connection_handler(LPVOID temp)
{
	char recvbuf[sizeof(Data)*2];
	int recvbuflen = 0;
    Data * bufdata = (Data *) recvbuf;
    int sock = *(int*)temp;
    char* buffer = 0;
    size_t n, bufferSize;

    for(;;){
        n = recv(sock, recvbuf + recvbuflen, sizeof(Data), NULL); 
        if (n < 0) {
			perror("ERROR reading from socket");
			closesocket(sock);
			shutdown(sock, 2);
			return 0;
        }

        if (n == 0) {
			perror("Connection closed");
			fflush(stdout);
			closesocket(sock);
			shutdown(sock, 2);
			return 0;
        }

        recvbuflen += n;

		if (bufdata->dataSize == 0) {
			buffer = (char *)realloc(buffer, bufferSize + 1);
			buffer[bufferSize] = 0;
			printf("Here is the message: %s\n", buffer);
            free(buffer);
		}
		else {
			if (buffer == 0){
				buffer = (char *)malloc(bufdata->dataSize);
				memcpy(buffer, bufdata->data, bufdata->dataSize);
				bufferSize = bufdata->dataSize;
			}

			else{
				buffer = (char *)realloc(buffer, bufferSize + bufdata->dataSize);
				memcpy(buffer + bufferSize, bufdata->data, bufdata->dataSize);
				bufferSize += bufdata->dataSize;
			}

			memcpy(recvbuf, recvbuf + bufdata->dataSize + 1, bufdata->dataSize + 1);
			recvbuflen -= bufdata->dataSize + 1;
		}
    }
    closesocket(sock);
	shutdown(sock, 2);
    return 0;
}
