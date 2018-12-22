#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <memory.h>
#include <string.h>
#include<arpa/inet.h>
#include<sys/socket.h>

#define PORT 8080

int main(void)
{
    char * bufData;

    printf("UDP Server\n");

    int my_sock;
    my_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (my_sock < 0)
    {
        perror("Creating socket error");
        exit(-1);
    }

    struct sockaddr_in local_addr;
    memset((char *) &local_addr, 0, sizeof(local_addr));

    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(PORT);

    if(setsockopt(my_sock, SOL_SOCKET, SO_BROADCAST, & (int) {1}, sizeof(int)) < 0) {
        perror("Setsockopt error");
        close(my_sock);
        shutdown(my_sock, 2);
        exit(-1);
    }

    if (bind(my_sock, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0)
    {
        perror("Bind error");
        close(my_sock);
        shutdown(my_sock, 2);
        exit(-1);
    }

    while (1)
    {

        struct sockaddr_in client_addr;

        int client_addr_size = sizeof(client_addr);

        int bsize = getData(my_sock, &bufData, (struct sockaddr *)&client_addr, client_addr_size);
        if (bsize < 0) {
            perror("recvfrom() error");
            close(my_sock);
            shutdown(my_sock, 2);
            exit(-1);
        }

        bufData[bsize] = 0;

        printf("Client to server:%s\n", &bufData[0]);

        sendData(my_sock, &bufData[0], bsize, (struct sockaddr *)&client_addr, client_addr_size);
        free(bufData);
    }

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
        if ((nsize = recvfrom(s, &buf[0], sizeof(buf) - 1, 0, client_addr, &size)) < 0)
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

