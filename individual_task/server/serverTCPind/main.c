//Server trade

#include <sys/types.h>
#include <netinet/in.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define PORT 5001
#define BUF_SIZE 256
#define MAIN_MENU "Hello!\n What do you want?\n 1.See lot titles\n 2.New lot *only for manager*\n 3.See online users (also rewrite userlist.txt)\n 4.Exit\n 5.End *only for manager* \n"
#define WELCOME "Please type who are you: (log your_login)\n"
#define LOTS "If you want to make a bet enter new bet, which will be higher than older\n ('bet lot_name lot_price')\n"
#define NEW_LOT "Please write name and price of the new lot ('lot lot_name lot_price')\n"
#define SUCCESS "The new lot has created!\n"
#define DENIED "You are not a manager\n"
#define NOTLOGIN "You must login before make this action\n"
#define COMMAND_COUNT 10


void *ClientHandler(void* socket); //Client handler
void *ServerHandler(void* empty); //Server handler
void SendErrorToClient(int socket); // Send error to client
void SentErrServer(char *s, int socket); //error handling
void NewLot(char name[], char price[], int socket); //make new lot
int DeleteClient(char name[]); //Delete client
int FindNumberByName(char *name); //Find number of thread by Name
char *FindNameBySocket(int socket); //Find name of client by socket
void WhoIsOnline(char* out); //Make list of online users
char *SetPrice(int lot, char buf[], int socket); //Set price for lot and make status message
void SendToClient(int socket, char* message); //Send message to client
void SendResults(); //Send results to all users
void EndTrade(); //Delete all users
int FindTitle(char title[]);
void LotDetail(int lot, int socket, char* out);
void DisconnectUser(int i);

int threads = -1; //threads counter
int lotCount = -1;
bool manager_count = false; // if manager online

//Command list
char kill_command[] = "kill";
char online_command[] = "usrs";
char shutdown_command[] = "off\n";

struct clients {
    char login[BUF_SIZE];
    int ip;
    int port;
    int s1; // socket for correctly specify the name
    bool manager; // if client is manager
} *users;

struct lot {
    char lotName[BUF_SIZE];
    int price;
    char winner[BUF_SIZE];
} *lots;

static char *commands[] = {
        "new",
        "log",
        "0",
        "1",
        "bet",
        "2",
        "lot",
        "3",
        "4",
        "5"
};



int servSocket;


void readn(int sk, char* buf) {
    char symb[1];
    for (int i = 0; ; i++) {
        if (recv(sk, symb, 1, 0) < 0) {
            perror("ERROR reading from socket");
            exit(1);
        }
        if (symb[0] == 10){
            break;
        }
        buf[i] = symb[0];
    }
}


void *ServerHandler(void* empty) {
    char text[BUF_SIZE]; //buffer
    //Getting text from keyboard
    while (1) {

        bzero(text, BUF_SIZE + 1);
        fgets(text, BUF_SIZE, stdin);

        char buf[4];
        memset(buf, 0, 4);
        strncat(buf, text, 4);
        if (strcmp(kill_command, buf)==0) {

            char name[]="";

            int i = 5;
           while ((text[i] != NULL) && (text[i] != '\n')) {
                name[i-5] = text[i];
                i++;
            }
                name[i-5]='\0';

                if (!DeleteClient(name)){
                    printf("All right \n");
                    DisconnectUser(users[FindNumberByName(name)].s1);
                    pthread_exit(NULL);
                }
                else
                    printf("Bad command\n");
           }


        if (strcmp(online_command, buf)==0) {
            char out[BUF_SIZE] = "";
            WhoIsOnline(out);
            FILE *userlist;
            userlist = fopen("userlist.txt","w");
            fprintf(userlist,out);
            fclose(userlist);
            printf(out);
        }


        if (strcmp(shutdown_command, buf)==0) {
            SendResults();
            EndTrade();
            exit(0);
        }
    }
}

void SendToClient(int socket, char* message) {
    ssize_t rc;
    rc = send(socket, message, strlen(message), 0);
    if (rc <= 0)
        perror("send call failed");
}

void *ClientHandler(void* arg) {

    int rc;
    int socket = (int) arg;
    bool isLogin = false;
    bool isManager = false;

    //Working
    while (1) {
        char buf[ BUF_SIZE ]; //Buffer
        char pick[5] = "";
        readn(socket, buf);
        int position = 0;
        while ((buf[position] != ' ') && (buf[position] != '\n') && (buf[position] != NULL) && (position != 5))
        {
            pick[position] = buf[position];
            position++;
        }
        int command = 0;
        while (command <= COMMAND_COUNT)
        {
            if (!strcmp(commands[command], pick))
                break;
            command++;
        }

        position++; // to step over
        switch (command) {
            case 0 : // new
            {
                SendToClient(socket, WELCOME);
                break;
            }
            case 1 : // login
            {
                int i = position;
                char name[BUF_SIZE] = "";
                while ((buf[i] != NULL) && (buf[i] != '\n')) {
                    name[i-position]=buf[i];
                    i++;
                }

                //check for manager
                if (!strcmp(name, "manager")) {
                    isManager = true;
                }

                //delete if manager already exist
                if (isManager && manager_count) {
                    printf("Client was Deleted. Manager already exist\n");
                    SendToClient(socket, "#Manager already exist\n");
                    threads--;
                    pthread_exit(NULL);
                }

                //saving login
                if (isManager) {
                    users[threads].manager = true;
                    manager_count = true;
                }
                isLogin = true;
                strncpy(users[threads].login, name, strlen(name));
                SendToClient(socket, MAIN_MENU);
                printf("New user login is: %s\n", users[threads].login);
                printf("\n");
                break;

            }
            case 2: //Menu
            {
                SendToClient(socket, MAIN_MENU);
                break;
            }
            case 3: //See lot titles
            {
                if (!isLogin)
                {
                    SendToClient(socket, NOTLOGIN);
                    break;
                }
                char out[BUF_SIZE] = LOTS;
                for (int i = 0; i <= lotCount; i++) {
                    LotDetail(i, socket, out);
                }
                if (lotCount<0) {
                    char* emptyMessage = "Empty set\n";
                    strncat(out, emptyMessage, strlen(emptyMessage));
                }
                else
                {
                    strncat(out, "\n", 1);
                }
                SendToClient(socket, out); //send lot names
                break;

            }
            case 4: // bet lot_name lot_price
            {
                if (!isLogin)
                {
                    SendToClient(socket, NOTLOGIN);
                    break;
                }
                int i = position;
                char name[BUF_SIZE] = "";
                char price[BUF_SIZE] = "";
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n'))
                {
                    name[i-position] = buf[i];
                    i++;
                }
                i++;
                int j = 0;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n'))
                {
                    price[j] = buf[i];
                    i++;
                    j++;
                }
                int lot = FindTitle(name);
                if (lot == -1) {
                    SendToClient(socket, "No lot with this name \n");
                    SendToClient(socket, MAIN_MENU);
                    break;
                }
                SendToClient(socket, SetPrice(lot, price, socket));
                SendToClient(socket, MAIN_MENU);
                break;
            }
            case 5://New lot
            {
                if (!isLogin)
                {
                    SendToClient(socket, NOTLOGIN);
                    break;
                }
                if (isManager == false) {
                    SendToClient(socket, DENIED);
                    break;
                }

                SendToClient(socket, NEW_LOT);
                break;
            }
            case 6:
            {
                if (!isLogin)
                {
                    SendToClient(socket, NOTLOGIN);
                    break;
                }
                if (isManager == false) {
                    SendToClient(socket, DENIED);
                    break;
                }
                int i = position;
                char name[BUF_SIZE] = "";
                char price[BUF_SIZE] = "";
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n'))
                {
                    name[i-position] = buf[i];
                    i++;
                }
                i++;
                int j = 0;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n'))
                {
                    price[j] = buf[i];
                    i++;
                    j++;
                }

                NewLot(name, price, socket);
                SendToClient(socket, SUCCESS);
                SendToClient(socket, MAIN_MENU);
                break;
            }
            case 7://see online users
            {

                if (!isLogin)
                {
                    SendToClient(socket, NOTLOGIN);
                    break;
                }
                char out[BUF_SIZE] = " ";
                WhoIsOnline(out);
                SendToClient(socket, out);
                SendToClient(socket, MAIN_MENU);
                break;
            }
            case 8://Disconnect client
            {
                if (!isLogin)
                {
                    SendToClient(socket, NOTLOGIN);
                    break;
                }
                printf("%d\n", socket);
                if (isManager)
                    manager_count = false;
                DeleteClient(FindNameBySocket(socket));
                DisconnectUser((int)socket);
                pthread_exit(NULL);
                break;
            }
            case 9://See result
            {
                if (!isLogin)
                {
                    SendToClient(socket, NOTLOGIN);
                    break;
                }
                if (!isManager) {
                    SendToClient(socket, DENIED);
                    break;
                }
                SendResults();
                EndTrade();
                exit(0);

            }
            default://if client type illegal point in main menu
            {
                SendErrorToClient(socket);
            }
        }
        memset(buf, 0, BUF_SIZE);
    }
}

void LotDetail(int lot, int socket, char* out) {

    strncat(out, lots[lot].lotName, strlen(lots[lot].lotName));
    strncat(out, " ", 1);
    char priceBuf[BUF_SIZE] = "";
    sprintf(priceBuf, "%d", lots[lot].price);
    strncat(out, priceBuf, strlen(priceBuf));
    strncat(out, " ", 1);
    strncat(out, lots[lot].winner, strlen(lots[lot].winner));
    strncat(out, "\n", 1);
}

int FindTitle(char title[]) {
    for (int i = 0; i <= threads; i++)
        if (!strcmp(lots[i].lotName, title))
            return i;
    return -1;
}

void SendErrorToClient(int socket) {
    int rc = 0;
    rc = send((int) socket, "^", 2, 0);
    if (rc <= 0)
        perror("send call failed");
}

int DeleteClient(char name[]) {
    printf("DeleteClient running...\n");
    int number;
    number = FindNumberByName(name);
    if (number != -1) {
        if (users[number].manager)
            manager_count = false;
        SendToClient(users[number].s1, "#");
        printf("\n The client %s was deleted \n", users[number].login);
        if (number != threads) {
            users[number] = users[threads];
            memset(&users[threads], NULL, sizeof (users[threads]));
        }

        return 0;
    }
    return 1;
}

int FindNumberByName(char *name) {

    for (int j = 0; j <= threads; j++) {
        if (!strcmp(users[j].login, name))
            return j;
    }
    return -1;
}

void WhoIsOnline(char* out) {
    strncat(out, "You want to see online users:\n", 30);
    //Show logins
    for (int i = 0; i <= threads; i++) {
        strncat(out, users[i].login, strlen(users[i].login));
        strncat(out, "\n", 1);
    }
}

char *SetPrice(int lot, char buf[], int socket) {
    int price = -1;
    price = atoi(buf);
    if (price <= 0) {
        return "New price has wrong format\n";
    }
    if (price <= lots[lot].price) {
        return "New price <= old price!\n";
    } else {
        printf("last_price=%d\n", price);
        lots[lot].price = price;
        char* winner =  FindNameBySocket(socket);
        memset(lots[lot].winner, 0, BUF_SIZE);
        strncpy(lots[lot].winner, winner, strlen(winner));
        return "All right!\n";
    }

}

char *FindNameBySocket(int socket) {
    for (int i = 0; i <= threads; i++) {
        if (users[i].s1 == socket)
            return users[i].login;
    }
    return "error";
}

void SentErrServer(char *s, int socket) //error handling
{
    perror(s);
    DeleteClient(FindNameBySocket(socket));

}

void NewLot(char name[], char price[], int socket) {
    int newPrice = -1;
    newPrice = atoi(price);
    if (newPrice <= 0 || strlen(name) <= 0) {
        SendToClient(socket, "Invalid data \n");
        return;
    }
    strncpy(lots[lotCount + 1].lotName, name, strlen(name));
    lots[lotCount + 1].price = newPrice;
    char* winner =  FindNameBySocket(socket);
    strncpy(lots[lotCount + 1].winner, winner, strlen(winner));
    printf("\n New lot %s created by %s with price %d \n", name, winner, newPrice);
    lotCount++;
}

void SendResults() {

    for (int i = 0; i <= threads; i++) {
        char result[BUF_SIZE] = "%";
        for (int j = 0; j <= lotCount; j++) {
            if (!strcmp(lots[j].winner, users[i].login)) {

                char out[BUF_SIZE] = "You win lot ";
                strcat(out, lots[j].lotName);
                strcat(out, " with price ");
                char priceBuf[BUF_SIZE] = "";
                sprintf(priceBuf, "%d", lots[j].price);
                strcat(out, priceBuf);
                strcat(out, "!\n");
                strcat(result, out);
            } else {

                char out_loser[BUF_SIZE] = "You lose lot ";
                strcat(out_loser, lots[j].lotName);
                strcat(out_loser, " with price ");
                char priceBuf[BUF_SIZE] = "";
                sprintf(priceBuf, "%d", lots[j].price);
                strcat(out_loser, priceBuf);
                strcat(out_loser, "!\n");
                strcat(out_loser, "The winner is ");
                strcat(out_loser, lots[j].winner);
                strcat(out_loser, "!\n");
                strcat(result, out_loser);
            }
        }
        SendToClient( users[i].s1, result);
    }
}

void DisconnectUser(int i) {
   //     SendToClient(users[i].s1, "#");
        shutdown(users[i].s1, 2);
        close(users[i].s1);
}

void EndTrade() {
    for (int i = 0; i <= threads; i++) {
        shutdown(users[i].s1, 2);
        close(users[i].s1);
    }
    close(servSocket);

}

int main(void) {
    users = (char*) malloc(BUF_SIZE*10);
    if (users == NULL) {
        printf("Cant create struct for users");
        EndTrade();
        exit(1);
    }

    lots = (char*) malloc(BUF_SIZE*10);
    if (lots == NULL) {
        printf("Cant create struct for lots");
        EndTrade();
        exit(1);
    }

    printf("Server trade is working...\n");

    //Initialization
    struct sockaddr_in local, si_other;
    int s1, rc, slen = sizeof (si_other);

    //fill local
    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    //make socket
    servSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (servSocket < 0)
        printf("Socket call failed");

    //attach port
    rc = bind(servSocket, (struct sockaddr *) &local, sizeof (local));
    if (rc < 0)
        printf("Bind call failure");



    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    //thread for server
    pthread_t server_thread;
    pthread_create(&server_thread, &threadAttr, ServerHandler, (void*) NULL);

    //listening socket
    rc = listen(servSocket, 5);
    if (rc)
        printf("Listen call failed");

    while (1) {
        //get connection
        s1 = accept(servSocket, (struct sockaddr *) &si_other, &slen);
        if (s1 < 0)
            printf("Accept call failed");

        //Making new user struct
        users[threads + 1].ip = inet_ntoa(si_other.sin_addr);
        users[threads + 1].port = ntohs(si_other.sin_port);
        users[threads + 1].s1 = s1;
        printf("new socket=%d\n", (int) s1);

        //New thread for client
        pthread_t client_thread;
        pthread_create(&client_thread, &threadAttr, ClientHandler, (void*) s1);
        threads++;
    }
    return 0;
}