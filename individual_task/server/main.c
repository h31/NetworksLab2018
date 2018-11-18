#include "sockets/sockets.h"
#include "logging/logging.h"
#include "files/files.h"

int console_log = 1;

FILE* file_to_log;

// Function for listening 5001 port and accepting connections
void *listen_func (void *arg);

// Function for messaging with client
void *client_func (void *arg);

int main() {
	pthread_t listen_thread; // Thread for listening
	int listen_socket; // Socket for listening
	int res; // Result of functions
	char key; // Key pressed
	
	log_init("server.log");
	mlog("Staring server");
	
	listen_socket = create_listen_socket();
	mlogf("listen socket fd = %d", listen_socket);
	res = pthread_create(&listen_thread, NULL, listen_func, &listen_socket); // Create thread for listening
	if (res != 0) {
		close_socket(listen_socket, "ERROR while creating listen thread");
	}
	
	while(key != 'q') {
		key = getchar();
		switch(key) {
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

	/*char token[50];
	int res;
	res = authentication("login", "passwd");
	if(res != FILES_OK) {
		mlog("Client is not exists");
	}
	
	res = verify(token);
	if(res != FILES_OK) {
		mlog("User not verified");
	}
	
	res = register_client("login", "passwd");
	if(res != FILES_OK) {
		mlogf("register not ok, err=%d", res);
		return 0;
	} 
	res = verify(token);
	if(res != FILES_OK) {
		mlog("User not verified");
	}
	mlog("Registred, now login");
	res = login("login", token, 50);
	if(res != FILES_OK) {
		mlogf("login not ok, err=%d", res);
	}
	res = verify(token);
	if(res != FILES_OK) {
		mlog("User not verified");
	}*/
/*
	struct command comm;
	struct request req;
	char a[4];
	a[0] = 0x10;
	a[1] = 0x00;
	a[2] = 0x00;
	a[3] = 0x00;
	int i;
	bcopy(a, &(req.comm.type), 4);
	printf("%d\n", req.comm.type);
	*/
	
	
	/*
	char* buf = malloc(10*sizeof(char));
	printf("start %p\n", buf);
	read_from(1, buf, 10);
	printf("end 2 %p\n", buf);
	printf("%s\n", buf); */
	
	/*
	struct sockaddr_in cli_addr;
	unsigned int clilen;
	int sockfd, newsockfd;
	int i;
	
	clilen = sizeof(cli_addr);
	sockfd = create_listen_socket();
	printf("listen socket created fd = %d\n", sockfd);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	printf("accepted fd = %d\n", newsockfd);*/
	
	/*
	struct request req;
	//int res = read_from(newsockfd, buf, 16*sizeof(char));
	int res = get_request(newsockfd, &req);
	printf("res=%d\n", res);
	if(res != OK) return res;
	//int res = read_from(newsockfd, buf, sizeof(int));
	//printf("res = %d\n", res);
	//printf("%s\n", buf);
	//printf("NEW:\n");
	//int i = *(int *)buf;
	//printf("%d\n", i);

	printf("\nMAIN:\n");
	printf("type=%d\n", req.comm.type);
	printf("arg1=%s\n", req.comm.arg1);
	printf("arg2=%s\n", req.comm.arg2);
	printf("token=%s\n", req.token);*/
	
	//pthread_t thread1; //thread for messaging
	//pthread_t thread2; //thread for messaging
	//int result;	//result of thread creating
	//result = pthread_create(&thread1, NULL, thread1_func, NULL); //create new thread
	//result = pthread_create(&thread2, NULL, thread2_func, NULL); //create new thread
	//sleep(1);
	//mylog("end");
}

void *listen_func (void *arg) {
	int sockfd = * (int *) arg; // Listen socket
	int newsockfd; // Accepted socket for client
	
	struct sockaddr_in cli_addr;
	unsigned int clilen = sizeof(cli_addr);
	
	// Accept connections
	while(1) {
		// Accept new connection
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0) {
			mlog("ERROR on accept");
			pthread_exit(1);
		}
		
		mlogf("New connection accepted with fd = %d", newsockfd);
		
		// Making new thread for messaging with client 
		pthread_t client_thread; // Thread for messaging
		int result;	// Result of thread creating
		result = pthread_create(&client_thread, NULL, client_func, &newsockfd); // Create new thread
		if(result != 0) {
			close_socket(newsockfd, "Error while creating client thread");
		}
	}
	pthread_exit(0);
}

void *client_func (void *arg) {
	int sockfd = * (int *) arg; // Client socket
	struct request req; // Request from client
	struct response resp; // Response to client
	int res;
	
	while(req.comm.type != QUIT) {
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
		
		// Handle request
		if(strcmp(req.comm.type, "GET") == 0) {
			mlog("GET request");
			//if(req.token == NULL) {
				//resp.type = "
			//res = verify(req.token);
		} else if (strcmp(req.comm.type, "REG") == 0) {
			mlogf("REG request; arg1=%s; arg2=%s", req.comm.arg1, req.comm.arg2);
		} else if (strcmp(req.comm.type, "LOGIN") == 0) {
			mlog("LOGIN request");
		} else if (strcmp(req.comm.type, "LIST") == 0) {
			mlog("LIST request");
		} else if (strcmp(req.comm.type, "SEND") == 0) {
			mlog("SEND request");
		} else if (strcmp(req.comm.type, "QUIT") == 0) {
			mlog("QUIT request");
		} else if (strcmp(req.comm.type, "DEL") == 0) {
			mlog("DEL request");
		} else {
			mlogf("Unknow type of request: %s", req.comm.type);
		}
	}
	close_socket(sockfd, "no errors");
	pthread_exit(0);
}

