#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <unistd.h>
#include <string.h>
#define  SHUT_RDWR 2


DWORD WINAPI mainFunc();
DWORD WINAPI communic(LPVOID temp);

volatile BOOL flag = TRUE;

int main(int argc, char *argv[]) {
	WSADATA WSStartData; 
	if (WSAStartup(MAKEWORD(2, 2), &WSStartData) != 0) {
		perror("ERROR on WSAStartup");
		exit(1);
	}
	CreateThread(NULL, 0, mainFunc, NULL, 0, NULL);
	while(getchar() != 'q'){
	}
	return 0;
}


DWORD WINAPI mainFunc() { 
    SOCKET sockfd;
    int portno;
    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
	

    if (sockfd == INVALID_SOCKET) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

	if(setsockopt(sockfd, SOL_SOCKET,  SO_REUSEADDR, & (int) {1}, sizeof(int)) < 0) {
	    	perror("ERROR on setsockopt");
	    	shutdown(sockfd, SHUT_RDWR);
	    	closesocket(sockfd);
		exit(1);
    }

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");

		shutdown(sockfd, SHUT_RDWR);
		closesocket(sockfd);
        exit(1);
    }

    listen(sockfd, 5);
    	while(1) {
		if (flag == TRUE) {
			flag = FALSE;
			CreateThread(NULL, 0, communic, &sockfd, 0, NULL);
		}
	}
}

DWORD WINAPI communic(LPVOID temp) {
    unsigned int clilen;
	struct sockaddr_in cli_addr;
	clilen = sizeof(cli_addr);
	int sockfd = *(int *) temp;
	long messlen;
	char buffer[256];
	int n;
	
	SOCKET newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	flag = TRUE;
	if (newsockfd == INVALID_SOCKET) {
	    perror("ERROR on accept");

		shutdown(sockfd, SHUT_RDWR);
		closesocket(sockfd);
	    exit(1);
	}
		
	/* If connection is established then start communicating */
	memset(buffer, 0, 256);
	n = recv(newsockfd, buffer, 255, 0); 

	if (n < 0) {
		perror("ERROR lenght of message");

		shutdown(sockfd, SHUT_RDWR);
		closesocket(sockfd);

		shutdown(newsockfd, SHUT_RDWR);
		closesocket(newsockfd);
		exit(1);
	}
	
	messlen = atol(buffer);

	memset(buffer, 0,  256);

	for(int i = 0; i < messlen; i += n) {
		n = recv(newsockfd, buffer + i, 255, 0); 
		if (n < 0) {
			perror("ERROR of message");

			shutdown(sockfd, SHUT_RDWR);
			closesocket(sockfd);

			shutdown(newsockfd, SHUT_RDWR);
			closesocket(newsockfd);
			exit(1);
		}
	}
	
	printf("Here is the message: %s\n", buffer);

	/* Write a response to the client */
	n = send(newsockfd, "I got your message", 18, 0); 

	if (n < 0) {
		perror("ERROR writing to socket");

		shutdown(sockfd, SHUT_RDWR);
		closesocket(sockfd);

		shutdown(newsockfd, SHUT_RDWR);
		closesocket(newsockfd);
		exit(1);
	}
	closesocket(newsockfd);
	return 0;
} 