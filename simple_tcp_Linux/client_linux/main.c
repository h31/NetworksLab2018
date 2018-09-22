#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>

typedef struct Data_s{
	char dataSize;
	char data[256];
} Data;

char buffer[65536];

int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);

    /* Now connect to the server */
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(1);
    }

    /* Now ask for a message from the user, this message
       * will be read by server
    */

    printf("Please enter the message: ");
    bzero(buffer, 65536);
    fgets(buffer, 65535, stdin);

    ssize_t size = strlen(buffer);
    ssize_t iter = 0;
    Data data;
    int rem; 

    while((rem = (int)size - iter) > 0){
	if (rem > 255) data.dataSize = 255;
	else data.dataSize = rem;
	memcpy(data.data, buffer, data.dataSize);
	int result = send(sockfd, (char*)&data, (int)data.dataSize + 1, NULL);
	if (result <= 0) {
	    perror("ERROR connection lost");
            exit(2);
	}
	iter += data.dataSize;
	usleep(0);
    }

    data.dataSize = 0;
    send(sockfd, (char*)&data, 1, NULL);
    return 0;
}
