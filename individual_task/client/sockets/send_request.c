#include "sockets.h"

int send_request(int sockfd, struct request *req) {
  int type_length;  // Length of request type
  int arg1_length;  // Length of first argument of command
  int arg2_length;  // Length of second argument of command
  int token_length; // Length of token
  int length;       // Total length of request
  char *buf;        // Wrap request into this buffer
  int buf_pointer = 0;
  int res;

  // Set length of type
  type_length = strlen(req->comm.type);

  // Set length of arg1
  if (req->comm.arg1 != NULL) {
    arg1_length = strlen(req->comm.arg1);
  } else {
    arg1_length = 0;
  }

  // Set length of arg2
  if (req->comm.arg2 != NULL) {
    arg2_length = strlen(req->comm.arg2);
  } else {
    arg2_length = 0;
  }

  // Set length of token
  if (req->token != NULL) {
    token_length = strlen(req->token);
  } else {
    token_length = 0;
  }

  // Count total length of request
  length =
      sizeof(int) * 3 +
      (type_length + arg1_length + arg2_length + token_length) * sizeof(char);

  // Send length to server
  res = write(sockfd, &length, sizeof(int));
  if (res < 0) {
    close_socket(sockfd, "ERROR write length to socket");
    return -1;
  }

  // Allocate memory for request
  buf = (char *)malloc(length);
  bzero(buf, length);

  // Set request type length
  bcopy(&type_length, &buf[buf_pointer], sizeof(int));
  buf_pointer += sizeof(int);

  // Set request type
  bcopy(req->comm.type, &buf[buf_pointer], type_length * sizeof(char));
  buf_pointer += type_length * sizeof(char);

  // Set length of arg1
  bcopy(&arg1_length, &buf[buf_pointer], sizeof(int));
  buf_pointer += sizeof(int);

  // Set arg1
  if (arg1_length > 0) {
    bcopy(req->comm.arg1, &buf[buf_pointer], arg1_length * sizeof(char));
    buf_pointer += arg1_length * sizeof(char);
  }

  // Set length of arg2
  bcopy(&arg2_length, &buf[buf_pointer], sizeof(int));
  buf_pointer += sizeof(int);

  // Set arg1
  if (arg2_length > 0) {
    bcopy(req->comm.arg2, &buf[buf_pointer], arg2_length * sizeof(char));
    buf_pointer += arg2_length * sizeof(char);
  }

  // Set token
  bcopy(req->token, &buf[buf_pointer], token_length * sizeof(char));

  // check
  /*
  int i;
  printf("request bytes:\n");
  for(i = 0; i < length; i++) {
          printf("req: %X\n", buf[i]);
  }*/

  // Send request to server
  res = write(sockfd, buf, length);
  if (res < 0) {
    close_socket(sockfd, "ERROR write request to socket");
    return -1;
  }

  return OK;
}
