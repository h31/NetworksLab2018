#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

int main(int argc, char *argv[]) {
    int sockfd, n;
    uint16_t portno;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    int salen = sizeof(serv_addr);

    char buffer[256];

    if (argc < 3) 
    {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    portno = (uint16_t) atoi(argv[2]);

    /* Create a socket point */
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (sockfd == -1) 
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

    bzero((char *) &serv_addr, salen);
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *) &serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons(portno);


    /* Now ask for a message from the user, this message
       * will be read by server
    */
    while(1)
    {
    	printf("Please enter the message: ");
    	bzero(buffer, 256);
	fgets(buffer, 255, stdin); 

        /* Send message to the server */
    	n = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &serv_addr, salen);

    	if (n == -1) 
	{
        	perror("ERROR writing to socket");
        	exit(1);
        }

    	/* Now read server response */
   	bzero(buffer, 256);
    	/*n = read(sockfd, buffer, 255); from TCP
	if (n < 0) {
        perror("ERROR reading from socket");   from TCP
        exit(1);
   	 }
   	*/

    	n = recvfrom(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &serv_addr, &salen);

    	if (n == -1) 
        {
	perror("ERROR accepting data from socket");
	exit(1);
        }

        printf("%s\n", buffer);
    }
    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);
    return 0;
}