#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <signal.h>

#define IMITATE_PARTLY_SENDING

int sockfd;

void closeApp() {
	printf("Closing socket\r\n");
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	exit(0);
}

int main(int argc, char *argv[]) {
	signal(SIGINT, closeApp);
	int n;
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
	
	int textInputSize;
	textInputSize = 0;
	
	while (1) {
		printf("Please enter the message: ");
		bzero(buffer, 256);
		fgets(buffer + 1, sizeof(buffer) - 2, stdin);
		
		if (strstr(buffer + 1, "\\q") != NULL) {
			closeApp();
		}
			
		textInputSize = strlen(buffer + 1);
		printf("Message size = %d\r\n", textInputSize);
		buffer[0] = textInputSize;
		
		/* Send message to the server */
		#ifdef IMITATE_PARTLY_SENDING
			int firstPartSize = 4;
			n = write(sockfd, buffer, firstPartSize);
			n = write(sockfd, buffer + firstPartSize, textInputSize + 2 - firstPartSize);
		#else
			n = write(sockfd, buffer, textInputSize + 2);
		#endif

		if (n < 0) {
			perror("ERROR writing to socket");
			closeApp();
		}

		/* Now read server response */
		bzero(buffer, sizeof(buffer));
		n = read(sockfd, buffer, sizeof(buffer) - 1);

		if (n < 0) {
			perror("ERROR reading from socket");
			closeApp();
		}

		printf("%s\n", buffer);
	}
}