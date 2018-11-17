//#include "main.h"
#include "sockets/sockets.h"
#include "logging/logging.h"

void *thread1_func (void *arg) {
	int a;
	for(a = 0; a < 10; a++) {
		printf("thread1: %d\n", a);
		//pthread_t thId = pthread_self();
		printf("%d", getpid());
		mylog("thread1");
	}
	pthread_exit(0);
}

void *thread2_func (void *arg) {
	int b;
	for(b = 0; b < 10; b++) {
		printf("thread2: %d\n", b);
		mylog("thread2");
	}
	pthread_exit(0);
}

#define TEST 1

int main() {
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
	
	clilen = sizeof(cli_addr);
	sockfd = create_listen_socket();
	printf("listen socket created fd = %d\n", sockfd);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	printf("accepted fd = %d\n", newsockfd);
	
	char* buf = malloc(sizeof(int));
	printf("memory allocated\n");
	int res = read_from(newsockfd, buf, sizeof(int));
	printf("res = %d\n", res);
	printf("%s\n", buf);
	printf("NEW:\n");
	int i = *(int *)buf;
	printf("%d\n", i);
	for(i=0; i<4;i++) {
		printf("%X\n", buf[i]);
	}*/
	
	//pthread_t thread1; //thread for messaging
	//pthread_t thread2; //thread for messaging
	//int result;	//result of thread creating
	//result = pthread_create(&thread1, NULL, thread1_func, NULL); //create new thread
	//result = pthread_create(&thread2, NULL, thread2_func, NULL); //create new thread
	//sleep(1);
	//mylog("end");
	return 0;
}

