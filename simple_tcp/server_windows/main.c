#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define  SHUT_RDWR 2

volatile BOOL flag = TRUE;

DWORD WINAPI waitFunc();
DWORD WINAPI commFunc(LPVOID temp);
void closeSocket(SOCKET sock);
char* readMessage (SOCKET sock, SOCKET newsock);
void writeMessage (SOCKET newsock, char* buffer);

int main(int argc, char *argv[]) {
	/*Initialize  library wsock32.dll*/
	WSADATA WsaData; 
	int  WsaError; 
	
	WsaError = WSAStartup(0x0101, &WsaData);
	
	if (WsaError!=0) {
		perror("ERROR on WSAStartup");
		exit(1);
	}
	
	/* Create thread for waiting of request */
	CreateThread(NULL, 0, waitFunc, NULL, 0, NULL);
	while(getchar() != 'q') {
	}
	
	return 0;
}

/* Function to create socket and wait for requests*/
DWORD WINAPI waitFunc() { 
    SOCKET sockfd;
    uint64_t portno;
    struct sockaddr_in serv_addr;
	BOOL temp;
	

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
	
	temp = TRUE;

	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &temp, sizeof(BOOL)) < 0) {
	    	perror("ERROR on setsockopt");
	    	closeSocket(sockfd);
		exit(1);
    }

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
		closeSocket(sockfd);
		
        exit(1);
    }
	
    /* Now start listening for the clients*/
    listen(sockfd, 5);
    	while(1) {
		if (flag == TRUE) {
			flag = FALSE;
			CreateThread(NULL, 0, commFunc, &sockfd, 0, NULL);
		}
	}
}

/*Function for handling request*/
DWORD WINAPI commFunc(LPVOID temp) {
    int clilen;
	struct sockaddr_in cli_addr;
	int sockfd;

	char* buffer = (char*)calloc(256, sizeof(char));

	clilen = sizeof(cli_addr);
	sockfd = *(int *) temp;
	
	 /* Accept actual connection from the client */
	SOCKET newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	flag = TRUE;
	if (newsockfd == INVALID_SOCKET) {
	    perror("ERROR on accept");
		closeSocket(sockfd);
		
	    exit(1);
	}
	 /*Get the message from client*/
	buffer = readMessage(sockfd, newsockfd);
	printf("Here is the message: %s\n", buffer);	
	free(buffer);
	
	 /*Write a response  to the client*/
	writeMessage(newsockfd, "I GOT YOUR MESSAGE");
	
	closesocket(newsockfd);
	return 0;
} 

void closeSocket(SOCKET sock) {
	shutdown(sock, SHUT_RDWR);
	closesocket(sock);
}

char* readMessage (SOCKET sock, SOCKET newsock) {
	int n;
	uint32_t messlen;
	char* buffer = (char*)calloc(256, sizeof(char));
	char* bufForLen = (char*)calloc(4, sizeof(char));
	
	n = recv(newsock, bufForLen, 4, 0); 

	if (n < 0) {
		perror("ERROR lenght of message");
		closeSocket(sock);
		closeSocket(newsock);
		
		exit(1);
	}
	
	messlen = atol(bufForLen);
	free(bufForLen);

	for(unsigned int i = 0; i < messlen; i += n) {
		n = recv(newsock, buffer + i, 255, 0); 
		if (n < 0) {
			perror("ERROR of message");
			closeSocket(sock);
			closeSocket(newsock);
			
			exit(1);
		}
	}
	return buffer;
}

void writeMessage (SOCKET newsock, char* buffer) {
	int n;
	uint32_t messlen;
    char* bufForLen = (char*)calloc(4, sizeof(char));
	
    messlen = strlen(buffer);
    sprintf(bufForLen, "%04d", messlen);
    n = send(newsock, bufForLen, strlen(bufForLen), 0);

    if (n < 0) {
        perror("ERROR writing to socket length of message");
        closeSocket(newsock);
        
        exit(1);
    }
    free(bufForLen);
	
	n = send(newsock, buffer, messlen, 0); 

	if (n < 0) {
		perror("ERROR writing to socket");
		closeSocket(newsock);
		
		exit(1);
	}
}