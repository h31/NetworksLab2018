
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>

#define SRV_IP "127.0.0.1"
#define PORT 5001
#define BUF_SIZE 256

struct sockaddr_in peer;
int id = -1;
int messageId = 0;

void SentErr(char *s) //error handling
{
    perror(s);
    exit(1);
}

void *SendHandler(void* socket) {

    int s = (int) socket;
    int rc;
    printf("Type 'new' if you want to connect\n");
    while (1) {
        char text[BUF_SIZE] = "";
        fgets(text, BUF_SIZE, stdin);
        if (id != -1) {
            text[strlen(text)] = id+48;
        }
        char messId[BUF_SIZE];
        messId[0]=(messageId/10)+'0';
        messId[1]=(messageId%10)+'0';
        messId[2]=" ";
        strcat(messId,text);
        rc = sendto(s, messId, BUF_SIZE, 0, (struct sockaddr *) &peer, sizeof (peer));
        if (rc <= 0)
            SentErr("Sent call error");
        messageId++;
        memset(messId,0,BUF_SIZE);
    }

}

int main(void) {
    struct hostent *server;
    int s;
    int rc;
    //Fill sockaddr_in
    peer.sin_family = AF_INET;
    peer.sin_port = htons(PORT);
    peer.sin_addr.s_addr = inet_addr(SRV_IP);
    //we set up sockaddr_in
    if (inet_aton(SRV_IP, &peer.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    //Get socket
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0)
        SentErr("Socket call failed");
    //Making connection

    server = gethostbyname("localhost");
    bcopy(server->h_addr, (char *) &peer.sin_addr.s_addr, (size_t) server->h_length);
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);
    pthread_t send_thread;

    rc = pthread_create(&send_thread, &threadAttr, SendHandler, (void*) s);
    char buf[ BUF_SIZE ];
    struct sockaddr_in serv_two;
    int serv_two_size = sizeof (serv_two);

    while (1) {

        int rc = recvfrom(s, buf, BUF_SIZE, 0, (struct sockaddr *) &serv_two, &serv_two_size);
        if (rc <= 0)
            SentErr("Recive call failed");
        else {
            int i = 0;
            while (buf[i] != NULL) {
                if (buf[0] == '^')//If we doing something wrong
                {
                    printf("Invalid choose.Press enter and Try again\n");
                }
                if (buf[0] == '#')//If we was disconected
                {
                    printf("Closing connection...\n");
                    return 0;
                }
                printf("%c", buf[i]);
                i++;
            }
            if (strstr(buf, "id")) {
                id = buf[2]-48;
            }
            printf("\n____________________\n");
        }
        memset(buf, 0, BUF_SIZE);
    }
    return 0;
}

