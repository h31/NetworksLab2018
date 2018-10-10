#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <windows.h>
#include <winsock2.h>

#include <stdint.h>					// uint16_t
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define bcopy(b1,b2,len) (memmove((b2), (b1), (len)), (void) 0)


int main(int argc, char *argv[]) {
	WSADATA wsaData;

	unsigned int t;
	t = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (t != 0) {
		printf("WSAStartup failed: %ui\n", t);
		exit(1);
	}

	SOCKET sockfd;
	int	n;
	uint16_t portno;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}

	portno = (uint16_t)atoi(argv[2]);

	/* Create a socket point */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == INVALID_SOCKET) {
		perror("ERROR opening socket");
		exit(1);
	}

	server = gethostbyname(argv[1]);

	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr, (size_t)server->h_length);
	serv_addr.sin_port = htons(portno);

	/* Now connect to the server */
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		perror("ERROR connecting");
		exit(1);
	}

	/* Now ask for a message from the user, this message
	   * will be read by server
	*/

	char * buffer = NULL;
	size_t m_length;
	printf("Please enter the message: ");
	if ((m_length = getline(&buffer, &m_length, stdin) == -1)) {
		perror("Error reading your message");
		exit(1);
	}

	/* Send message to the server */
	n = send(sockfd, buffer, strlen(buffer), 0);

	if (n < 0) {
		perror("ERROR writing to socket");
		exit(1);
	}

	/* Now read server response */
	bzero(buffer, 256);

	while (1) {
		n = recv(sockfd, buffer, 255, 0); // recv on Linux
		printf("%s\n", buffer);
		bzero(buffer, 256);
		if (n < 255) break;
	}
	printf("---End of the response---\n");

	closesocket(sockfd);
	WSACleanup();

	return 0;
}
