#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

void closeSocket (int sock);

int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char* buffer;
    
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

    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, & (int) {1}, sizeof(int)) < 0) {
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

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */
    clilen = sizeof(cli_addr);

    if(fork() > 0){
        while(getchar() != 'q'){
        }
        closeSocket(sockfd);        
    }

    while(1) {

        n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *) &cli_addr, &clilen);

        if (n < 0) {
            perror("ERROR reading from socket");
            closeSocket(sockfd);
            
            exit(1);
        }

        printf("Here the message: %s\n", buffer);

        n = sendto(sockfd, "I GOT YOUR MESSAGE", 18, 0, (struct sockaddr *) &cli_addr, &clilen);

        if (n < 0) {
            perror("ERROR sending to socket");
            closeSocket(sockfd);
            
            exit(1);
        }

        
    }
    
    closeSocket(sockfd);
    free(buffer);

    return 0;
}

void closeSocket (int sock) {

    shutdown(sock, SHUT_RDWR);
    close(sock);

}