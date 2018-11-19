#include "files/files.h"
#include "logging/logging.h"
#include "sockets/sockets.h"

int console_log = 1;

FILE *file_to_log;

// Function for listening 5001 port and accepting connections
void *listen_func(void *arg);

// Function for messaging with client
void *client_func(void *arg);

int main() {
  pthread_t listen_thread; // Thread for listening
  int listen_socket;       // Socket for listening
  int res;                 // Result of functions
  char key;                // Key pressed

  log_init("server.log");
  mlog("Staring server");

  // Create listen socket
  listen_socket = create_listen_socket();
  mlogf("listen socket fd = %d", listen_socket);
  // Create thread for listening
  res = pthread_create(&listen_thread, NULL, listen_func, &listen_socket);
  if (res != 0) {
    close_socket(listen_socket, "ERROR while creating listen thread");
  }

  while (key != 'q') {
    key = getchar();
    switch (key) {
    case 'l': // Turn on/off logging to console
      console_log = !console_log;
      break;
    case 'q': // Quit
      mlog("Shutting down server");
      break;
    }
  }
  close_socket(listen_socket, "no errors");

  log_close();
  sleep(1);
  return 0;
}

void *listen_func(void *arg) {
  int sockfd = *(int *)arg; // Listen socket
  int newsockfd;            // Accepted socket for client

  struct sockaddr_in cli_addr;
  unsigned int clilen = sizeof(cli_addr);

  // Accept connections
  while (1) {
    mlog("Waiting for connections");
    // Accept new connection
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0) {
      mlog("ERROR on accept");
      pthread_exit(1);
    }

    mlogf("New connection accepted with fd = %d", newsockfd);

    // Making new thread for messaging with client
    pthread_t client_thread; // Thread for messaging
    int result;              // Result of thread creating
    result = pthread_create(&client_thread, NULL, client_func,
                            &newsockfd); // Create new thread
    if (result != 0) {
      close_socket(newsockfd, "Error while creating client thread");
    }
  }
  pthread_exit(0);
}

void *client_func(void *arg) {
  int sockfd = *(int *)arg; // Client socket
  struct request req;       // Request from client
  int res = 0;

  while (1) {
    mlog("Waiting for request");
    res = get_request(sockfd, &req);

    // Handle errors
    switch (res) {
    case ERROR_READING_FROM_SOCKET:
      close_socket(sockfd, "ERROR while reading from socket");
      pthread_exit(1);
      break;
    case READING_IS_NOT_FINISHED:
      close_socket(sockfd, "Client closed connection");
      pthread_exit(1);
      break;
    case REQUEST_LENGTH_ERROR:
      close_socket(sockfd, "ERROR in request length");
      pthread_exit(1);
      break;
    }

    // Log request
    mlogf("Type: %s", req.comm.type);
    mlogf("Arg1: %s", req.comm.arg1);
    mlogf("Arg2: %s", req.comm.arg2);
    mlogf("Token: %s", req.token);

    // Handle request
    if (strcmp(req.comm.type, "GET") == 0) {
      // GET REQUEST
      if (req.token == NULL) {
        // Client is not logged
        send_response(sockfd, RESPONSE_ERROR, "You are not logged");
        continue;
      }
      // Get client money
      char login[50];
      // int money = -1;
      bzero(login, 50);
      get_login(req.token, &login);
      mlogf("Login = %s", login);
      res = get_money(login);
      mlogf("Money = %d", res);
      if (res < 0) {
        send_response(sockfd, RESPONSE_ERROR, "Error on reading data");
        continue;
      }
      char s[12];
      sprintf(s, "%d", res);
      send_response(sockfd, RESPONSE_OK, s);
    } else if (strcmp(req.comm.type, "REG") == 0) {
      // REG REQUEST
      // Check if arguments are NULL
      if (req.comm.arg1 == NULL || req.comm.arg2 == NULL) {
        // Illegal arguments
        mlog("ERROR. Illegal arguments");
        send_response(sockfd, RESPONSE_ERROR, "Illegal arguments");
        continue;
      }
      // Check if user already exists
      res = authentication(req.comm.arg1, req.comm.arg2);
      if (res == 0 || res == WRONG_PASSWORD) {
        // User exists
        mlog("ERROR. User already exists");
        send_response(sockfd, RESPONSE_ERROR, "User already exists");
        continue;
      }
      // Register new client (create file in CLIENTS_FOLDER)
      res = register_client(req.comm.arg1, req.comm.arg2);
      if (res != 0) {
        mlogf("ERROR. Code = %d", res);
        send_response(sockfd, RESPONSE_ERROR, "Error on registration");
        continue;
      }
      // Login client and return session token
      char token[50];
      res = login_client(req.comm.arg1, token, 50);
      if (res != 0) {
        mlogf("ERROR. Code = %d", res);
        send_response(sockfd, RESPONSE_ERROR, "Error on login");
        continue;
      }
      send_response(sockfd, RESPONSE_TOKEN, token);
    } else if (strcmp(req.comm.type, "LOGIN") == 0) {
      // LOGIN REQUEST
      // Check if user exists
      res = authentication(req.comm.arg1, req.comm.arg2);
      if (res == USER_NOT_FOUND) {
        // User not exists
        mlog("ERROR. User not found");
        send_response(sockfd, RESPONSE_ERROR, "User not found");
        continue;
      } else if (res == WRONG_PASSWORD) {
        // Password is wrong
        mlog("ERROR. Wrong password");
        send_response(sockfd, RESPONSE_ERROR, "Wrong password");
        continue;
      }
      // Login client and return session token
      char token[50];
      res = login_client(req.comm.arg1, token, 50);
      if (res != 0) {
        mlogf("ERROR. Code = %d", res);
        send_response(sockfd, RESPONSE_ERROR, "Error on login");
        continue;
      }
      send_response(sockfd, RESPONSE_TOKEN, token);
    } else if (strcmp(req.comm.type, "LIST") == 0) {
      // LIST REQUEST
      char list[1024];
      list_all(list);
      send_response(sockfd, RESPONSE_OK, list);
    } else if (strcmp(req.comm.type, "SEND") == 0) {
      mlog("SEND request");
      send_response(sockfd, RESPONSE_ERROR, "Not implemented yet");
    } else if (strcmp(req.comm.type, "QUIT") == 0) {
      // QUIT REQUEST
      // Check if token is NULL
      if (req.token == NULL) {
        // Client is not logged
        send_response(sockfd, RESPONSE_ERROR, "You are not logged");
        continue;
      }
      // Delete client token drom "database"
      res = delete_client_token(req.token);
      if (res != 0) {
        mlogf("ERROR. Delete_token code = %d", res);
        send_response(sockfd, RESPONSE_ERROR, "Error on deleting data");
        continue;
      }
      // Send responce that we have deleted token. Now user must delete his
      // token, too
      send_response(sockfd, RESPONSE_OK, "Token deleted");
    } else if (strcmp(req.comm.type, "DEL") == 0) {
      // DEL REQUEST
      // Check if token is NULL
      if (req.token == NULL) {
        // Client is not logged
        send_response(sockfd, RESPONSE_ERROR, "You are not logged");
        continue;
      }
      // Get login of client by token
      char login[50];
      res = get_login(req.token, login);
      if (res != 0) {
        mlogf("ERROR. Code = %d", res);
        send_response(sockfd, RESPONSE_ERROR, "Error on get login");
        continue;
      }
      int res2;
      // Delete client token and login
      res = delete_client_token(req.token);
      res2 = delete_client_data(login);
      if (res != 0 || res2 != 0) {
        mlogf("ERROR. Delete_token code = %d; Delete_login code = %d", res,
              res2);
        send_response(sockfd, RESPONSE_ERROR, "Error on deleting data");
        continue;
      }
      send_response(sockfd, RESPONSE_DELETED, "Data deleted");
    } else {
      mlogf("Unknow type of request: %s", req.comm.type);
      send_response(sockfd, RESPONSE_ERROR, "Unknown type of request");
    }
  }
  close_socket(sockfd, "no errors");
  pthread_exit(0);
}

// Function for sending response to client
void send_response(int sockfd, char *type, char *payload) {
  int res;
  res = internal_send_response(sockfd, type, payload);
  if (res == ERROR_WRITING_TO_SOCKET) {
    close_socket(sockfd, "ERROR writing to socket");
    pthread_exit(1);
  }
}
