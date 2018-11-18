#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <unistd.h>
#include <stdint.h>

#include <string.h>
#define SHUT_RDWR 2

void closeSocket(SOCKET sock);
void writeMessage(SOCKET sock, char* buffer);
char* readMessage(SOCKET sock);

int main(int argc, char *argv[]) {
    SOCKET sockfd;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char* buffer = (char*)calloc(256, sizeof(char));
	
	/*Initialize  library wsock32.dll*/
	WSADATA WsaData; 
	int  WsaError;
	WsaError = WSAStartup(0x0101, &WsaData);
	
	if (WsaError!=0) {
		perror("ERROR on WSAStartup");
		exit(1);
	}
	
	if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (int) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd == INVALID_SOCKET) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
		closeSocket(sockfd);

        exit(0);
    }
	
	/* Initialize socket structure */
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
	memmove((char *)&serv_addr.sin_addr.s_addr, server->h_addr, (size_t)server->h_length); 
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        perror("ERROR connecting");
		closeSocket(sockfd);

        exit(1);
    }

    /* Now ask for a message from the user, this message will be read by server */
    printf("Please enter the message: ");
    fgets(buffer, 255, stdin);

    /* Send message to the server */
   writeMessage(sockfd, buffer);
   free(buffer);

    /* Now read server response */
    buffer = readMessage(sockfd);

	closeSocket(sockfd);
	free(buffer);

    return 0;
}

void closeSocket(SOCKET sock) {
	shutdown(sock, SHUT_RDWR);
	closesocket(sock);
}

void writeMessage (SOCKET sock, char* buffer) {

    uint32_t messlen;
    char* bufForLen = (char*)calloc(4, sizeof(char));
	int n;

    /* Send length of message to the server */
    messlen = strlen(buffer);
    sprintf(bufForLen, "%04d", messlen);
    n = send(sock, bufForLen, strlen(bufForLen), 0);

    if (n < 0) {
        perror("ERROR writing to socket length of message");
        closeSocket(sock);
        
        exit(1);
    }
    free(bufForLen);

    /* Send message to the server */
    n = send (sock, buffer, messlen, 0);

    if (n < 0) {
        perror("ERROR writing to socket message");
        closeSocket(sock);
        
        exit(1);
    }
}

char* readMessage(SOCKET sock) {

    char* buffer = (char*)calloc(256, sizeof(char));
    char* bufForLen = (char*)calloc(4, sizeof(char));
    uint32_t messlen;
	int n;

    /*Read lenght of message from the server*/
    n = recv(sock, bufForLen, 4, 0); 

    if (n < 0) {
        perror("ERROR lenght of message");
        closeSocket(sock);
        
        exit(1);
    }
    
    messlen = atol(bufForLen);
    free(bufForLen);

    /*Read message from the server*/
    for(unsigned int i = 0; i < messlen; i += n) {

    n = recv(sock, buffer + i, 255, 0); 

        if (n < 0) {
            perror("ERROR of message");
            closeSocket(sock);
            
            exit(1);
        }
    }

    printf("%s\n", buffer);

    return buffer;
}
