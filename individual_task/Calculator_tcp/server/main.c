// socket server example, handles multiple clients using threads
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>    
#include <stdlib.h>    
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <unistd.h>    
#include <pthread.h>
#include <dirent.h> 
#include <assert.h>
#include <math.h>
#include <errno.h>

#define MAX_BUFFER_SIZE 256
#define MAX_PATH_SIZE 512
#define MAX_OPS_CNT 64
#define MAX_CLIENT_ID 50
#define MAX_CONNECTIONS 100
#define MAX_SERVER_CMD 256

int getSO_ERROR(int fd) {
    int err = 1;
    socklen_t len = sizeof err;
    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&err, &len)) {
        perror("getSO_ERROR");
        exit(EXIT_FAILURE);
    }
    if (err) {
        errno = err;
    }
    return err;
}

void closeSocket(int fd) {
    if (fd > 0) {
        getSO_ERROR(fd);
        if (shutdown(fd, SHUT_RDWR) < 0) {
            if (errno != ENOTCONN && errno != EINVAL) {
                perror("shutdown");
                exit(EXIT_FAILURE);
            }
        }
        if (close(fd) < 0) {
            perror("close");
            exit(EXIT_FAILURE);
        }
    }
}

//the thread function
void *connection_handler(void *);
void *long_operation_handler(void *longOpInfo);

pthread_mutex_t lock, randLock;

typedef struct {
    int sock;
    char clientID[MAX_CLIENT_ID];
    bool clientIDUsed;
    bool shouldBeClosed;
} Connection;

Connection connections[MAX_CONNECTIONS];
int connectionCnt;

void *listener(void *notUsed);
pthread_mutex_t conLock;

void closeSocket(int fd);


int main(int argc , char *argv[])
{
    connectionCnt = 0;
    if (pthread_mutex_init(&conLock, NULL)) {
        perror("Failed to initialize the mutex");
        return EXIT_FAILURE;
    }
    pthread_t listenerTID;
    if (pthread_create(&listenerTID, NULL, listener, NULL) < 0) {
        perror("Failed to create listener thread");
        return EXIT_FAILURE;
    }
    char cmd[MAX_SERVER_CMD];
    printf("Enter command: ");
    while (fgets(cmd, MAX_SERVER_CMD, stdin)) {
        int cmdLen = strlen(cmd);
        if (cmdLen == 0 || cmd[cmdLen - 1] != '\n') {
            puts("Bad input");
            continue;
        }
        cmd[cmdLen - 1] = '\0';
        if (!strcmp(cmd, "quit")) {
            break;
        }
        if (!strcmp(cmd, "ls")) {
            pthread_mutex_lock(&conLock);
            int i;
            for (i = 0; i < connectionCnt; i++) {
                if (connections[i].clientIDUsed) {
                    puts(connections[i].clientID);
                }
            }
            pthread_mutex_unlock(&conLock);
        } else if (strstr(cmd, "disconnect ") == cmd) {
            char *clientName = cmd + strlen("disconnect ");
            pthread_mutex_lock(&conLock);
            int i;
            for (i = 0; i < connectionCnt; i++) {
                if (connections[i].clientIDUsed && !strcmp(connections[i].clientID, clientName)) {
                    connections[i].shouldBeClosed = true;
                    break;
                }
            }
            pthread_mutex_unlock(&conLock);
        } else {
            puts("Unknown command");
        }
        printf("Enter command: ");
    }
    pthread_mutex_destroy(&conLock);
    return EXIT_SUCCESS;
}

void *listener(void *notUsed) {
    (void*)notUsed;

    pthread_detach(pthread_self());
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        perror("Could not create socket");
		exit(EXIT_FAILURE);
    }
    //puts("Socket created");

    int on = 1;
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 7000 );

    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        exit(EXIT_FAILURE);
    }
    //puts("bind done");

    //Listen
    listen(socket_desc, 1000);

    //Accept and incoming connection
    //puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
	
	if (pthread_mutex_init(&lock, NULL) != 0) {
		perror("mutex init failed\n");
		exit(EXIT_FAILURE);
	}
    if (pthread_mutex_init(&randLock, NULL)) {
        perror("Failed initialize mutex");
        exit(EXIT_FAILURE);
    }
    while ((client_sock = accept(socket_desc,(struct sockaddr*)&client,(socklen_t*)&c)))
    {
        //puts("Connection accepted");

        pthread_t tid;
        new_sock = (int*)malloc(sizeof(int));
        *new_sock = client_sock;

        pthread_mutex_lock(&conLock);
        connections[connectionCnt].clientIDUsed = false;
        connections[connectionCnt].sock = client_sock;
        connections[connectionCnt].shouldBeClosed = false;
        connectionCnt++;
        pthread_mutex_unlock(&conLock);
        if (pthread_create(&tid, NULL, connection_handler, new_sock) < 0)
        {
            perror("could not create thread");
            exit(EXIT_FAILURE);
        }

        //puts("Handler assigned");
    }
	
	pthread_mutex_destroy(&lock);

    if (client_sock < 0)
    {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    return 0;
}

enum Command {
	CMD_UNKNOWN = 0,
	CMD_QUIT,
	CMD_ADD,
	CMD_SUB,
	CMD_MUL,
	CMD_DIV,
	CMD_FACT,
	CMD_SQRT,
	CMD_HEARTBIT,
    CMD_SETID
};

double doQuickOperation(enum Command cmd, double op1, double op2) {
	if (cmd == CMD_ADD) {
		return op1 + op2;
	}
	if (cmd == CMD_SUB) {
		return op1 - op2;
	}
	if (cmd == CMD_MUL) {
		return op1 * op2;
	}
	if (cmd == CMD_DIV) {
		return op1 / op2;
	}
	return 0.;
}

double getFact(long op) {
	if (op == 0 || op == 1) {
		return 1;
	}
	return op * getFact(op - 1);
}

double doLongOperation(enum Command cmd, double op) {
	assert(cmd >= CMD_FACT || cmd <= CMD_SQRT);
	if (cmd == CMD_FACT) {
		return getFact(lround(op));
	}
	return sqrt(op);
}

typedef struct {
	pthread_t tid;
	enum Command cmd;
	double arg;
} LongOpInfo;

typedef struct {
	pthread_t tid;
	enum Command cmd;
	double arg;
	double res;
    int time;
} LongOpResultInfo;

LongOpResultInfo longOpResults[MAX_OPS_CNT];
int resStart = 0;
int resEnd = 0;

void addLongOperation(enum Command cmd, double op) {
	LongOpInfo *opInfo = (LongOpInfo *) malloc(sizeof(LongOpInfo));
	opInfo->tid = pthread_self();
	opInfo->cmd = cmd;
	opInfo->arg = op;
	pthread_t hThread;
	if (pthread_create(&hThread, NULL, long_operation_handler, opInfo) < 0) {
		perror("Failed to create a thread\n");
		exit(2);
	}
}

// returns true if we need to continue processing commands
// after the command
bool processCommand(int sock) {
    fd_set set;
    struct timeval tv;
    int rv;

    bool weShouldClose = false;
    while (1) {
        FD_ZERO(&set);
        FD_SET(sock, &set);
        tv = (struct timeval){0};
        tv.tv_sec = 0;
        tv.tv_usec = 500;
        rv = select(sock + 1, &set, NULL, NULL, &tv);
        if (rv == -1) {
            return false;
        }
        if (rv == 0) {
            pthread_mutex_lock(&conLock);
            int i;
            for (i = 0; i < connectionCnt; i++) {
                if (connections[i].sock == sock) {
                    if (connections[i].shouldBeClosed) {
                        weShouldClose = true;
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&conLock);
            if (weShouldClose) {
                return false;
            }
            continue;
        }
        uint32_t cmdAsInt;
        unsigned receiveCnt;
        receiveCnt = recv(sock, &cmdAsInt, sizeof(uint32_t), 0);
        if (receiveCnt != sizeof(uint32_t)) {
            return false;
        }
        enum Command cmd = cmdAsInt;
        if (cmd >= CMD_ADD && cmd <= CMD_DIV) {
            double ops[2];
            receiveCnt = recv(sock, ops, sizeof(double)<<1, 0);
            if (receiveCnt != (sizeof(double)<<1)) {
                return false;
            }
            double result = doQuickOperation(cmd, ops[0], ops[1]);
            unsigned sentCnt = send(sock, &result, sizeof(double), 0);
            return sentCnt == sizeof(double);
        } else if (cmd == CMD_QUIT) {
            return false;
        } else if (cmd == CMD_FACT || cmd == CMD_SQRT) {
            double op;
            receiveCnt = recv(sock, &op, sizeof(double), 0);
            if (receiveCnt != sizeof(double)) {
                return false;
            }
            addLongOperation(cmd, op);
            return true;
            //double result = doQuickOperation(cmd, ops[0], ops[1]);
            //unsigned sentCnt = send(sock, &result, sizeof(double), 0);
            //return receiveCnt == sizeof(double);
        } else if (cmd == CMD_SETID) {
            uint32_t clientIDLen;
            recv(sock, &clientIDLen, sizeof(uint32_t), 0);
            char clientID[MAX_CLIENT_ID];
            recv(sock, clientID, clientIDLen, 0);
            clientID[clientIDLen] = '\0';
            pthread_mutex_lock(&conLock);
            int i;
            for (i = 0; i < connectionCnt; i++) {
                if (!connections[i].clientIDUsed && connections[i].sock == sock) {
                    strcpy(connections[i].clientID, clientID);
                    connections[i].clientIDUsed = true;
                }
            }
            pthread_mutex_unlock(&conLock);
        } else if (cmd == CMD_HEARTBIT) {
            LongOpResultInfo res;
            bool found = false;
            pthread_mutex_lock(&lock);
            int i;
            for (i = resStart; i < resEnd; i++) {
                if (longOpResults[i].tid == pthread_self()) {
                    memcpy(&res, longOpResults + i, sizeof(LongOpResultInfo));
                    if (i != resStart) {
                        memcpy(longOpResults + i, longOpResults + resStart,
                            sizeof(LongOpResultInfo));
                    }
                    resStart++;
                    found = true;
                    break;
                }
            }
            pthread_mutex_unlock(&lock);
            if (found) {
                uint32_t cmdAsInt = (uint32_t) res.cmd;
                if (send(sock, &cmdAsInt, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    return false;
                }
                if (send(sock, &(res.arg), sizeof(double), 0) != sizeof(double)) {
                    return false;
                }
                if (send(sock, &(res.res), sizeof(double), 0) != sizeof(double)) {
                    return false;
                }
                if (send(sock, &(res.time), sizeof(int), 0) != sizeof(int)) {
                    return false;
                }
            } else {
                uint32_t cmdAsInt = (uint32_t) CMD_HEARTBIT;
                if (send(sock, &cmdAsInt, sizeof(uint32_t), 0) != sizeof(uint32_t)) {
                    return false;
                }
            }
        }
	    return true;
    }
    return false;
}

void *long_operation_handler(void *longOpInfoObj) {
	LongOpInfo *opInfo_ = (LongOpInfo*) longOpInfoObj;
	LongOpInfo opInfo;
	memcpy(&opInfo, opInfo_, sizeof(LongOpInfo));
	free(opInfo_);
	// we can use opInfo now
	double resValue = doLongOperation(opInfo.cmd, opInfo.arg);
    int randomValue;
    int minSeconds = 1;
    int maxSeconds = 10;
    pthread_mutex_lock(&randLock);
    randomValue = minSeconds + rand()%(maxSeconds - minSeconds + 1);
    pthread_mutex_unlock(&randLock);
	sleep((unsigned)randomValue);
	LongOpResultInfo res;
	res.tid = opInfo.tid;
	res.cmd = opInfo.cmd;
	res.arg = opInfo.arg;
	res.res = resValue;
    res.time = randomValue;
	pthread_mutex_lock(&lock);
	memcpy(longOpResults + resEnd, &res, sizeof(LongOpResultInfo));
	resEnd++;
	if (resEnd == MAX_OPS_CNT) {
		resEnd = 0;
	}
	pthread_mutex_unlock(&lock);
	return 0;
}

/*
  This will handle connection for each client
  */
void *connection_handler(void *socket_desc)
{
    pthread_detach(pthread_self());
	int* pSock = (int*)socket_desc;
	int sock = *pSock;
	free(pSock);

	while (processCommand(sock)) {
	}
    pthread_mutex_lock(&conLock);
    int t;
    for (t = 0; t < connectionCnt; t++) {
        if (connections[t].sock == sock) {
            if (t != connectionCnt - 1) {
                memcpy(&(connections[t]), &(connections[connectionCnt - 1]), sizeof(Connection));
            }
            connectionCnt--;
            break;
        }
    }
    pthread_mutex_unlock(&conLock);
	closeSocket(sock);
	//puts("Client Disconnected");
	return 0;
}
