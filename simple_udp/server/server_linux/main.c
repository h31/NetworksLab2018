#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include<pthread.h>
#include <string.h>

struct information 
{
	int sockfd;
	unsigned int clilen;
	struct sockaddr_in cli_addr; 
};
void *connection_handler(void *c);

int main(int argc, char *argv[]) {
    int sockfd, create_error;
    uint16_t portno;
    unsigned int clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;
    struct information sct; 
    pthread_t thread_num;

    /* First call to socket() function */
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))<0)
    {
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
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
    {
        perror("ERROR on binding");
	shutdown(sockfd, SHUT_RDWR); 
	close(sockfd);
        exit(1);
    }

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    //listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    sct.clilen = clilen;
    sct.sockfd = sockfd;
    sct.cli_addr = cli_addr;


    if((create_error = pthread_create(&thread_num , NULL , connection_handler , &sct)) != 0)
    {
    	perror("could not create thread");
	shutdown(sockfd, SHUT_RDWR); 
	close(sockfd); 
	exit(1); 
    }
    while(1) 
    { 
    }
}

void *connection_handler(void *c)
{
    //Get the socket descriptor
    //int sockt = *(int*)sockfd;
    int r_size,n;
    char *new_message , buffer[256];
    struct information sct = * (struct information *) c; 
  
    //Receive a message from client
    while(1)
    {
	if ((r_size = recvfrom(sct.sockfd , buffer , 256 , 0, (struct sockaddr *) &sct.cli_addr, &sct.clilen)) == -1)
	{
        	perror("ERROR reading from socket");
		shutdown(sct.sockfd, SHUT_RDWR); 
		close(sct.sockfd);
		pthread_exit(NULL);
        	exit(1);
    	}

    	printf("Here is the message: %s\n", buffer);

    	/* Write a response to the client */
	

        //end of string marker
	buffer[r_size] = '\0';
		
	//Send the message back to client
	//n=write(sockt, "I got your message", 18); // send on Windows
	if((n = sendto(sct.sockfd, buffer, r_size, 0, (struct sockaddr*) &sct.cli_addr, sct.clilen)) == -1)
	{
        	perror("ERROR sending to socket");
		shutdown(sct.sockfd, SHUT_RDWR); 
		close(sct.sockfd);
		pthread_exit(NULL);
        	exit(1);
    	}
	
		
	//clear the message buffer
	memset(buffer, 0, 256);
	
    }

        shutdown(sct.sockfd, SHUT_RDWR);
    	close(sct.sockfd);
	pthread_exit(NULL);
    	return 0;
} 
