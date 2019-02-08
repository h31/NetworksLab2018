#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string.h>

#include <pthread.h>

int sockfd;
#define max_clients 10
#define true 1
#define false 0
int cliPorts[max_clients];
int isActive[max_clients];	// bool

#define maxRecords 5
#define storyLength 3
#define nameLength 3

struct currencyData
{
	char name[nameLength];
	int value;
	int story[storyLength];
	int actual;
};
struct currencyData mainMass[maxRecords];

typedef struct {
	int socket;
	// May be extended later
} requestDataStruct;

int addNewPortToList(int port) {
	int i = 0;
	for (; i < max_clients; i = i + 1) {
		if (isActive[i] == false) {
			isActive[i] = true;
			cliPorts[i] = port;
			return i;
		}
	}
	printf("Can't create a new connection!");
	exit(3);
}

void closeClientConnections() {
	int i = 0;
	for (; i < max_clients; i = i + 1) {
		if (isActive[i] == true) {
			shutdown(cliPorts[i], 2);
			close(cliPorts[i]);
		}
	}
	printf("Client connections have been closed...\n");
}

void *handleRequestThread(void *requestData) {
	char buffer[256];
	char answerBuffer[256];
	int newsockfd, n, boolplace;

	requestDataStruct *rds = (requestDataStruct*) requestData;

	newsockfd = rds->socket;
	boolplace = addNewPortToList(newsockfd);

	while(1) {
		/* If connection is established then start communicating */
		bzero(buffer, 256);

		n = recv(newsockfd, buffer, 255, 0); // recv on Linux
		if (n < 1) break;
		// V Можно включить для отладки
		// printf("Here is the message: ||%s||\n", buffer);

		if (buffer[0] == 'q') break;

		createAnswer(buffer, answerBuffer);

		sleep(3); // Operation imitation

		/* Write a response to the client */
		n = send(newsockfd, answerBuffer, strlen(answerBuffer), 0); // send on Linux

		if (n < 0) {
		    perror("ERROR writing to socket");
		    exit(1);
		}
	}
	shutdown(newsockfd, 2);
	close(newsockfd);
	isActive[boolplace] = false;

	return 0;
}

// l - лист валют
// a - добавление валюты
// d - удаление валюты
// c - добавление курса валюты
// s - история налюты номер n
void createAnswer(char buffer[], char *answer) {
	bzero(answer, 256);
	char *saveptr;
	char *word;
	word = strtok_r(buffer, " ", &saveptr);

	if (word[0] == 'l') {		// '\n' - особенность работы strtok
		for (int iter=0; iter<maxRecords; iter=iter+1)
			if (mainMass[iter].actual == 1) {	
				char teChar[10];
				strcat(answer,"\n");
				sprintf(teChar, "%d", (iter+1));
				strcat(answer,teChar);
				strcat(answer,") ");
				strcat(answer,mainMass[iter].name);
				strcat(answer,": ");
				sprintf(teChar, "%d", mainMass[iter].value);
				strcat(answer,teChar);
				// Добавить изменение валюты, если оно было
				if (mainMass[iter].story[0] != 0) {
				strcat(answer," (");
				if (mainMass[iter].value > mainMass[iter].story[0])
					strcat(answer,"+");
				sprintf(teChar, "%d", (mainMass[iter].value - mainMass[iter].story[0]));
				strcat(answer,teChar);
				strcat(answer,")");
				}
			}
		strcat(answer,"\n");
		return;
	}

	if (!strcmp(word,"a")) {
		int flag = 0;		// Флаг успешности операции
		for (int iter=0; iter<maxRecords; iter=iter+1)
			if (mainMass[iter].actual == 0) {
				bzero(mainMass[iter].name, nameLength);
				word = strtok_r(NULL, " ", &saveptr);
				if (word == NULL) break;
				strncpy(mainMass[iter].name, word, 3);

				mainMass[iter].value = 0;
				
				int iter2;
				for (iter2=0; iter2<storyLength; iter2=iter2+1)
					mainMass[iter].story[iter2] = 0;

				mainMass[iter].actual = 1;

				flag = 1;

				break;
			}
			if (flag == 0) strcat(answer,"Error");
			else strcat(answer,"Success");
		return;
	}

	if (!strcmp(word,"d")) {
		// Узнать номер валюты
		word = strtok_r(NULL, " ", &saveptr);
		if (word == NULL) {
			strcat(answer,"Error. Please enter currency number");
			return;
		}
		int temp = atoi(word) - 1;
		if ((temp < 0) || (temp >= maxRecords)) {
			strcat(answer,"Error. Wrong currency number");
			return;
		}
		if (mainMass[temp].actual == 0) {
			strcat(answer,"Error. Currency does not exist");
			return;
		}
		// Удалить валюту
		mainMass[temp].actual = 0;
		strcat(answer,"Success");

		return;
	}

	if (!strcmp(word,"c")) {
		// Узнать номер валюты
		word = strtok_r(NULL, " ", &saveptr);
		if (word == NULL) {
			strcat(answer,"Error. Please enter currency number");
			return;
		}
		int temp = atoi(word) - 1;
		if ((temp < 0) || (temp >= maxRecords)) {
			strcat(answer,"Error. Wrong currency number");
			return;
		}
		if (mainMass[temp].actual == 0) {
			strcat(answer,"Error. Currency does not exist");
			return;
		}

		// Узнать значение курса
		word = strtok_r(NULL, " ", &saveptr);
		if (word == NULL) {
			strcat(answer,"Error. Please enter currency value");
			return;
		}
		int temp2 = atoi(word);
		if (temp2 <= 0) {
			strcat(answer,"Error. Wrong value format");
			return;
		}
		if (mainMass[temp].value == temp2) {
			strcat(answer,"Error. Same value");
			return;
		}
		
		// Записать результат
		int temp3 = mainMass[temp].value;
		int iter;
		for (iter=storyLength-1; iter>0; iter=iter-1)
			mainMass[temp].story[iter] = mainMass[temp].story[iter-1];
		mainMass[temp].story[0] = temp3;
		mainMass[temp].value = temp2;

		// Формирование ответа
		strcat(answer,"Success");

		return;
	}

	if (!strcmp(word,"s")) {
		// Узнать номер валюты
		word = strtok_r(NULL, " ", &saveptr);
		if (word == NULL) {
			strcat(answer,"Error. Please enter currency number");
			return;
		}
		int temp = atoi(word) - 1;
		if ((temp < 0) || (temp >= maxRecords)) {
			strcat(answer,"Error. Wrong currency number");
			return;
		}
		if (mainMass[temp].actual == 0) {
			strcat(answer,"Error. Currency does not exist");
			return;
		}
		// Сформировать ответ
		strcat(answer,mainMass[temp].name);
		strcat(answer,": ");
		char teChar[10];
		sprintf(teChar, "%d", mainMass[temp].value);
		strcat(answer,teChar);
		int iter;
		for (iter=0; iter<storyLength; iter=iter+1) {
			strcat(answer," - ");
			sprintf(teChar, "%d", mainMass[temp].story[iter]);
			strcat(answer,teChar);
		}
		return;
	}

	strcat(answer,"Unknown command. Try again");
}

void handleRequest(int socket) {
	requestDataStruct rds;
	int status;
	pthread_t aThread;

	rds.socket = socket;

	status = pthread_create(&aThread, NULL, handleRequestThread, (void*) &rds);
	if (status != 0) {
		printf("Cant create a thread. Status: %d\n", status);
		exit(2);
	}

	// Shouldnt wait for the end of the thread
}

void *controling(void *args) {
	char ch;

	do {
		ch = getchar();
	} while (ch != 'q');

	printf("Exiting...\n");
	shutdown(sockfd, 2);
	close(sockfd);

	return 0;
}

int main(int argc, char *argv[]) {

    int newsockfd;
    uint16_t portno;
    unsigned int clilen;
    struct sockaddr_in serv_addr, cli_addr;
    ssize_t n;

    /* First call to socket() function */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }

    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = 5001;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

	/* Creating a control thread */
	int status;
	pthread_t controlThread;

	status = pthread_create(&controlThread, NULL, controling, NULL);
	if (status != 0) {
		printf("Cant create a control thread. Status: %d\n", status);
		exit(2);
	}

    /* Now start listening for the clients, here process will
       * go in sleep mode and will wait for the incoming connection
    */

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    while(1) {
        /* Accept actual connection from the client */
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);	
		
        if (newsockfd < 0) {
            perror("ERROR on accept");
            break;
        }

		handleRequest(newsockfd);
    }

	closeClientConnections();
	printf("The server is off now\n\n");
	exit (0);
}
