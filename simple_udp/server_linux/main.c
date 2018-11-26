#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>


int main(int argc, char *argv[]) {
    int sockfd,received_size,ss;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }


    clilen = sizeof(cli_addr);

    while (1)
    {
        while((received_size = recvfrom(sockfd , buffer , 256 , 0,(struct sockaddr *) &cli_addr, &clilen)) > 0 )
        {
            //Send the message back to client
            if(sendto(sockfd , buffer , received_size,0,(struct sockaddr*) &cli_addr, clilen)<0)
            {
                perror("ERROR send");
                exit(1);
            }
            buffer[n] = '\0';
            printf("Here is the message: %s\n", buffer);
            memset(buffer ,'\0', 256);
            buffer[received_size] = '\0';
        }
        if(received_size == -1)
        {
            perror("ERROR recv");
            exit(1);
        }
        free(sockfd)
                close(sockfd);
        return 0;
    }
}





