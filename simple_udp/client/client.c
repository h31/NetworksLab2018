#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define DEFAULT_PORT 8080
#define SERVERADDR "127.0.0.1"



int main(int argc, char *argv[])
{

    struct sockaddr_in dest_addr;
    char buff[10 * 1014];
    printf("UDP Client\nType quit to quit\n");

    if (argc < 3) {
        perror("usage hostname port");
        exit(0);
    }

    int my_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (my_sock < 0)
    {
        perror("ERROR opening socket");
        exit(-1);
    }

    memset((char *) &dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(DEFAULT_PORT);

    if (inet_aton(SERVERADDR , &dest_addr.sin_addr) == 0)
    {
        perror("inet_aton() failed\n");
        close(my_sock);
        shutdown(my_sock, 2);
        exit(-1);
    }

    while (1)
    {
        printf("From client:"); fgets(&buff[0], sizeof(buff) - 1, stdin);
        if (!strcmp(&buff[0], "quit\n")) break;

        sendData(my_sock, &buff[0], strlen(&buff[0]), (struct sockaddr *)&dest_addr, sizeof(dest_addr));

        char * bufData = NULL;
        struct sockaddr_in server_addr;
        int server_addr_size = sizeof(server_addr);

        int bsize = getData(my_sock, &bufData, (struct sockaddr *)&server_addr, server_addr_size);

        if (bsize == 0)
        {
            perror("recvfrom error");
            close(my_sock);
            shutdown(my_sock, 2);
            exit(-1);
        }

        buff[bsize] = 0;

        printf("Server to client:%s", &buff[0]);
    }

    close(my_sock);
    shutdown(my_sock, 2);

    return 0;
}

void sendData(int s, char * data, int len, struct sockaddr * client_addr, int size)
{
    char * buff = (char*)calloc(len + sizeof(int), sizeof(char));

    memcpy(buff, &len, sizeof(int));
    memcpy(&buff[sizeof(int)], data, len);

    sendto(s, &buff[0], len + sizeof(int), 0, client_addr, size);

    free(buff);
}

int getData(int s, char ** buff, struct sockaddr * client_addr, int size)
{
    char buf[256];
    bzero(buf, 256);
    ssize_t nsize;
    int totalSize = 0;
    int getSize = -1;
    *buff = NULL;

    while (getSize < totalSize)
    {
        if ((nsize = recvfrom(s, &buf[0], sizeof(buf) - 1, 0, client_addr, &size)) == -1)
        {
            close(s);
            shutdown(s, 2);

            getSize = 0;

            if (*buff != NULL)
            {
                free(*buff);
                *buff = NULL;
            }
        }

        if (nsize > 0)
        {
            if (totalSize == 0)
            {
                getSize = nsize - sizeof(int);

                memcpy(&totalSize, &buf[0], sizeof(int));

                *buff = (char*)calloc(totalSize, sizeof(char));

                memcpy(*buff, &buf[sizeof(int)], getSize);
            }
            else
            {
                memcpy(*buff[getSize], &buf[sizeof(int)], nsize);
                getSize += nsize;
            }
        }
    }

    return getSize;
}

