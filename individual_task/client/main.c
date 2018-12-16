#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <pthread.h>
#include <cygwin/socket.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <asm/byteorder.h>
#include <cygwin/in.h>
#include <memory.h>
#include <netdb.h>
#include <stdbool.h>

int sockfd;

void closeApp() {
    printf("Closing socket\r\n");
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, closeApp);
    ssize_t n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];
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
    while (1) {
        bool isLog = false;

        bzero(buffer, 256);
        // Получение кода ответа сервера
        n = read(sockfd, buffer, 256);
        if (n <= 0) {
            perror("ERROR reading from socket");
            closeApp();
        }
        printf("%s\n", buffer);
        bzero(buffer, 256);
        // Получение ответа сервера
        n = read(sockfd, buffer, 256);
        if (n <= 0) {
            perror("ERROR reading from socket");
            closeApp();
        }
        printf("%s\n", buffer);
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);
        if (strstr(buffer, "\\q") != NULL) {
            closeApp();
        }
        /* Send message to the server */
        n = write(sockfd, buffer, strlen(buffer));
        if (n <= 0) {
            perror("ERROR writing to socket");
            closeApp();
        }

        /* Now read server response */
    }
    return 0;
}