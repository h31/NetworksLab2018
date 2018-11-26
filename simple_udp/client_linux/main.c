#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int sockfd, clilen;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    if (argc < 3)
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        exit(1);
    }

    server = gethostbyname(argv[1]);

    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);


    clilen = sizeof(serv_addr);


    while(1)
    {
        printf("Please enter the message: ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);

        /* Send message to the server */
        if( sendto(sockfd , buffer , 256 , 0,(struct sockaddr *) &serv_addr, clilen) < 0)
        {
            puts("ERROR sending");
            return 1;
        }
        memset(buffer,'\0', 256);
        /* Now read server response */
        if( recvfrom(sockfd , buffer , 256 , 0,(struct sockaddr *) &serv_addr, &clilen) < 0)
        {
            puts("ERROR recv");
            break;
        }

    }
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    return 0;
}
