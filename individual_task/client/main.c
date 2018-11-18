#include "sockets/sockets.h"

int main(int argc, char *argv[]) {
	struct request req; // Request to server
	int sockfd; // Socket for connection to server
	int res;
	char buf[256]; // Input buffer
	char* token; // Token for session
	
	// Clear token
	token = 0;
	
	// Connect to server
	sockfd = connect_to(argc, argv); 
	if(sockfd < 0) {
		exit(1);
	}
	printf("Connected to server with fd=%d\n", sockfd);
	
	while(1) {
		printf("Enter command>");
		
		// Get string from client
		bzero(buf, 256);
		fgets(buf, 256, stdin);
		res = parse_request(buf, &req);
		
		// Handle errors
		if(res == INVALID_TYPE) {
			printf("Invalid type of request!\n");
			break;
		}
		
		if(strcmp(req.comm.type, "QUIT") == 0 || strcmp(req.comm.type, "q") == 0) {
			break;
		}
		
		// Set token
		req.token = token;
		
		// Print request
		printf("Request:\n");
		printf("Type: %s\n", req.comm.type);
		printf("Arg1: %s\n", req.comm.arg1);
		printf("Arg2: %s\n", req.comm.arg2);
		printf("Token: %s\n", req.token);
		
		// Send request
		res = send_request(sockfd, &req);
	}
	
	close_socket(sockfd, "no errors");
	return 0;
}
