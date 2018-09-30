#include <stdio.h>

#include <WinSock2.h>
#include <string.h>
#include <thread>

using namespace std;

typedef struct Data_s{
	char dataSize;
	char data[256];
} Data;

void *connection_handler(void *);

int main(int argc, char *argv[]) {
    int sockfd, newsockfd;
    unsigned short portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
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
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    if(listen(sockfd, 5) < 0)
    {
	perror("listen");
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

void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    char* buffer = 0;
    size_t n, bufferSize;
    Data data;

    for(;;){
        n = recv(sock, (char*)&data, sizeof(Data), NULL); 
        if (n < 0) {
			perror("ERROR reading from socket");
			closesocket(sock);
			return 0;
        }

        if (n == 0) {
			perror("Connection closed");
			closesocket(sock);
			return 0;
        }

        if (n-1 != data.dataSize) {
			perror("ERROR packet");
			closesocket(sock);
			return 0;
        }

	if (data.dataSize == 0) {
	    buffer = (char *)realloc(buffer, bufferSize + 1);
            buffer[bufferSize] = 0;
            printf("Here is the message: %s\n", buffer);
            free(buffer);
	}
	else {
	    if (buffer == 0){
		buffer = (char *)malloc(data.dataSize);
		memcpy(buffer, data.data, data.dataSize);
		bufferSize = data.dataSize;
	    }

	    else{
		buffer = (char *)realloc(buffer, bufferSize + data.dataSize);
		memcpy(buffer + bufferSize, data.data, data.dataSize);
		bufferSize += data.dataSize;
	    }
	}
    }
    closesocket(sock);
    return 0;
}
