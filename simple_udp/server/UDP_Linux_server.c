#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include <memory.h>
 
#define BUFLEN 256
#define PORT 8080   
 
void die(char *s, int socket)
{
    perror(s);
    close(s);
    shutdown(s, 2);
    exit(1);
}
 
int main(void)
{
    struct sockaddr_in si_me, si_other;
     
    int s, i, slen = sizeof(si_other) , recv_len;
    char *buf;
     
    if ((s=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        die("socket", s);
    }
     
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    if(setsockopt(s, SOL_SOCKET, SO_BROADCAST, & (int) {1}, sizeof(int)) < 0) {
    	die("setsockopt", s);
    }
     
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) < 0)
    {
        die("bind", s);
    }
     
    while(1)
    {
        printf("Waiting for data...");
        buf = malloc(BUFLEN);
         
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) < 0)
        {
            die("recvfrom()", s);
        }

        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %s\n" , buf);
         
        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) < 0)
        {
            die("sendto()", s);
        }
    }
    
    close(s);
    shutdown(s, 2);
    return 0;
}
