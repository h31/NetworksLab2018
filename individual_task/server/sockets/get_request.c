#include "sockets.h"

int get_request(int sockfd, struct request *req) {
	int res; // Result of reading
	char* buf; // Buffer for reqding
	int message_length; // Length of message without first sizeof(int) bytes
	
	int arg_length;
	int buf_pointer = 0;

	// Get length of request
	buf = malloc(sizeof(int));
	res = read_from(sockfd, buf, sizeof(int));
	// Throw errors
	if (res != OK) {
		return res;
	}
	
	message_length = *(int *)buf;
	if (message_length <= 0 || message_length > 256) {
		return REQUEST_LENGTH_ERROR;
	}
	
	// Read byte array of request
	buf = malloc(message_length * sizeof(char));
	res = read_from(sockfd, buf, message_length);
	// Throw errors
	if (res != OK) {
		return res;
	}
	
	// Now convert byte array to request struct
	// Get type of request
	bcopy(&buf[buf_pointer], &(req->comm.type), sizeof(int));
	buf_pointer += sizeof(int);
	
	// Get length of arg1
	bcopy(&buf[buf_pointer], &arg_length, sizeof(int));
	buf_pointer += sizeof(int);
	
	// Get arg1
	req->comm.arg1 = (char*)malloc(arg_length * sizeof(char));
	bcopy(&buf[buf_pointer], (req->comm.arg1), arg_length * sizeof(char));
	buf_pointer += arg_length;
	
	// Get length of arg2
	bcopy(&buf[buf_pointer], &arg_length, sizeof(int));
	buf_pointer += sizeof(int);
	
	// Get arg2
	req->comm.arg2 = (char*)malloc(arg_length * sizeof(char));
	bcopy(&buf[buf_pointer], (req->comm.arg2), arg_length * sizeof(char));
	buf_pointer += arg_length;
	
	// Get token
	arg_length = message_length - buf_pointer;
	req->token = (char*)malloc(arg_length * sizeof(char));
	bcopy(&buf[buf_pointer], (req->token), arg_length * sizeof(char));

	return OK;
}

















