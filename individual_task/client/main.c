#include "sockets/sockets.h"

int main(int argc, char *argv[]) {
	struct request req; // Request to server
	struct response resp; // Response from server
	int sockfd; // Socket for connection to server
	int res;
	char buf[256]; // Input buffer
	char token[50]; // Token for session
	
	// Clear token
	bzero(token, 50);
	
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
		
		if(strcmp(req.comm.type, "q") == 0) {
			//req.token = token;
			//res = send_request(sockfd, &req);
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
		if(res < 0) {
			break;
		}
		
		// Read response
		res = get_response(sockfd, &resp);
		if (res < 0) {
			break;
		}
		// Print request
		printf("Response:\n");
		printf("Type: %s\n", resp.type);
		printf("Payload: %s\n", resp.payload);
		
		// Hadnle response
		if(strcmp(resp.type, "TOKEN") == 0) {
			strcpy(token, resp.payload);
			printf("I got token: %s\n", token);
		}
		
		if(strcmp(resp.type, "DELETED") == 0) {
			bzero(token, 50);
			printf("Token is set to zero\n");
		}
		
		if(strcmp(req.comm.type, "QUIT") == 0) {
			break;
		}
	}
	
	close_socket(sockfd, "no errors");
	return 0;
}
