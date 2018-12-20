//Server trade

#include <sys/types.h>
#include <sys/socket.h>
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
#define MAIN_MENU "Hello!\nWhat do you want?\n 1.See lot titles\n 2.New lot *only for manager*\n 4.Exit\n 5.End *only for manager*"
#define WELCOME "Please type who are you: (log your_login)"
#define LOTS "If you want to make a bet enter new bet, which will be higher than older ('bet lot_name lot_price')\n"
#define NEW_LOT "Please write name and price of the new lot ('lot lot_name lot_price')\n"
#define SUCCESS "The new lot has created!\n"
#define DENIED "You are not a manager\n"
#define NOTLOGIN "You must login before this action\n"
#define COMMAND_COUNT 10


void *ClientHandler(void* arg); //Client handler
void ServerHandler(); //Server handler
void SendErrorToClient(struct sockaddr_in cli_addr, unsigned int clilen); // Send error to client
void SentErrServer(char *s); //error handling
void NewLot(char name[], char price[], int userNumber); //make new lot
int FindNumberByName(char* name); //Find number of thread by Name
void WhoIsOnline(char* out); //Make list of online users
char *SetPrice(int lot, char buf[], int userNumber); //Set price for lot and make status message
void SendToClient(char* message, struct sockaddr_in cli_addr, unsigned int clilen); //Send message to client
void SendResults(); //Send results to all users
void EndTrade(); //Delete all users
int FindTitle(char title[]);
void LotDetail(int lot, char* out);
int id = 0;
int usersCount = -1; //users counter
int lotCount = -1;
bool manager_count = false; // if manager online

char shutdown_command[] = "shutdown\n";

struct clients {
    char login[BUF_SIZE];
    struct sockaddr_in cli_addr;
    bool manager; // if client is manager
    unsigned int clilen;
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


void ServerHandler() {

    //Getting text from keyboard
    while (1) {
        char text[40]; //buffer
        fgets(text, BUF_SIZE, stdin);

        char buf[4];
        if (!strcmp(shutdown_command, text)) {
            EndTrade();
            exit(1);
        }
    }
}

void SendToClient(char* message, struct sockaddr_in cli_addr, unsigned int clilen) {
    int rc;
    rc = sendto(servSocket, message, strlen(message), 0, (struct sockaddr*) &cli_addr, clilen);
    if (rc <= 0)
        perror("send call failed");
}




void *ClientHandler(void* arg) {

    int rc;

    int valid, prev=0;
    //Working
    while (1) {
        char buf[ BUF_SIZE ]; //Buffer
        char pick[5] = "";
        char idmes[2] = "";
        struct sockaddr_in cli_addr;
        unsigned int clilen;
        clilen = sizeof (cli_addr);
        rc = recvfrom(servSocket, buf, BUF_SIZE, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (rc <= 0)
            SentErrServer("Recv call failed");

       for(int i=0; i<2; i++) {
            idmes[i] = buf[i];
        }
       valid = ((int)idmes[0]-48)*10 + (int)idmes[1]-48;
       if(valid-prev>1)
           SendToClient("Some masage is lost. Try to reconnect\n",cli_addr,clilen);
       else
           prev=valid;

        int position = 3;
        while ((buf[position] != ' ') && (buf[position] != '\n') && (buf[position] != NULL) && (position != 8)) {
            pick[position-3] = buf[position];
            position++;
        }
        int command = 0;
        while (command <= COMMAND_COUNT) {
            if (!strcmp(commands[command], pick))
                break;
            command++;
        }

        position++; // to step over
        int userNumber = buf[strlen(buf)-1]-48;

        switch (command) {
            case 0: // new
            {
                char text[3] = "";
                sprintf(text, "id%d", id);
                SendToClient(text, cli_addr, clilen);
                SendToClient(WELCOME, cli_addr, clilen);
                id++;

                break;
            }
            case 1: // login
            {
                usersCount++;
                int i = position;
                char name[BUF_SIZE] = "";
                while ((buf[i] != NULL) && (buf[i] != '\n')) {
                    name[i - position] = buf[i];
                    i++;
                }

                //delete if manager already exist
                if (!strcmp(name, "manager")) {
                    if (manager_count == true) {
                        printf("Client was Deleted. Manager already exist\n");
                        SendToClient("#Manager already exist", cli_addr, clilen);
                        usersCount--;
                    }
                    else {
                        users[usersCount].manager = true;
                        manager_count = true;
                    }
                }


                strcpy(users[usersCount].login, name);
                SendToClient(MAIN_MENU, cli_addr, clilen);
                printf("New user login is: %s\n", users[usersCount].login);
                printf("\n");
                users[usersCount].cli_addr = cli_addr;
                users[usersCount].clilen = clilen;
                break;

            }
            case 2: //Menu
            {
                SendToClient(MAIN_MENU, cli_addr, clilen);
                break;
            }
            case 3: //See lot titles
            {
                if (strlen(users[userNumber].login) == 0)
                {
                    SendToClient(NOTLOGIN, cli_addr, clilen);
                    break;
                }

                char out[BUF_SIZE] = LOTS;
                for (int i = 0; i <= lotCount; i++) {
                    LotDetail(i, out);
                    strncat(out, "\n", 1);
                }
                SendToClient(out, cli_addr, clilen); //send lot names
                break;

            }
            case 4: // bet lot_name lot_price
            {
                if (strlen(users[userNumber].login) == 0)
                {
                    SendToClient(NOTLOGIN, cli_addr, clilen);
                    break;
                }
                int i = position;
                char name[BUF_SIZE] = "";
                char price[BUF_SIZE] = "";
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    name[i - position] = buf[i];
                    i++;
                }
                i++;
                int j = 0;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    price[j] = buf[i];
                    i++;
                    j++;
                }
                int lot = FindTitle(name);
                if (lot == -1) {
                    SendToClient("No lot with this name \n", cli_addr, clilen);
                    SendToClient(MAIN_MENU, cli_addr, clilen);
                    break;
                }
                SendToClient(SetPrice(lot, price, userNumber), cli_addr, clilen);
                SendToClient(MAIN_MENU, cli_addr, clilen);
                break;
            }
            case 5://New lot
            {
                if (strlen(users[userNumber].login) == 0)
                {
                    SendToClient(NOTLOGIN, cli_addr, clilen);
                    break;
                }
                if (users[userNumber].manager == false) {
                    SendToClient(DENIED, cli_addr, clilen);
                    break;
                }

                SendToClient(NEW_LOT, cli_addr, clilen);
                break;
            }
            case 6:
            {
                if (strlen(users[userNumber].login) == 0)
                {
                    SendToClient(NOTLOGIN, cli_addr, clilen);
                    break;
                }
                if (users[userNumber].manager == false) {
                    SendToClient(DENIED, cli_addr, clilen);
                    break;
                }
                int i = position;
                char name[BUF_SIZE] = "";
                char price[BUF_SIZE] = "";
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    name[i - position] = buf[i];
                    i++;
                }
                i++;
                int j = 0;
                while ((buf[i] != NULL) && (buf[i] != ' ') && (buf[i] != '\n')) {
                    price[j] = buf[i];
                    i++;
                    j++;
                }

                NewLot(name, price, userNumber);
                SendToClient(SUCCESS, cli_addr, clilen);
                SendToClient(MAIN_MENU, cli_addr, clilen);
                break;
            }
            case 7://see online users
            {
                /*
                                char out[BUF_SIZE] = " ";
                                WhoIsOnline(out);
                                SendToClient(out, cli_addr);
                                SendToClient(MAIN_MENU, cli_addr);
                 */
                break;
            }
            case 8://Disconnect client
            {
                if (users[userNumber].manager == true)
                    manager_count = false;
                SendToClient("#", cli_addr, clilen);
            }
            case 9://See result
            {
                if (strlen(users[userNumber].login) == 0)
                {
                    SendToClient(NOTLOGIN, cli_addr, clilen);
                    break;
                }
                if (users[userNumber].manager == false) {
                    SendToClient(DENIED, cli_addr, clilen);
                    break;
                }
                SendResults();
                EndTrade();
            }
            default://if client type illegal point in main menu
            {
                SendErrorToClient(cli_addr, clilen);
            }
        }
        memset(buf, 0, BUF_SIZE);
    }
}

void LotDetail(int lot, char* out) {

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
    for (int i = 0; i <= usersCount; i++)
        if (!strcmp(lots[i].lotName, title))
            return i;
    return -1;
}

void SendErrorToClient(struct sockaddr_in cli_addr, unsigned int clilen) {
    int rc = 0;
    rc = sendto(servSocket, "^", BUF_SIZE, 0, (struct sockaddr*) &cli_addr, clilen);
    if (rc <= 0)
        perror("send call failed");
}

int FindNumberByName(char* name) {

    for (int j = 0; j <= usersCount; j++) {
        if (!strcmp(users[j].login, name))
            return j;
    }
    return -1;
}

void WhoIsOnline(char* out) {
    strncat(out, "You want to see online users:\n", strlen("You want to see online users:\n"));
    //Show logins
    for (int i = 0; i <= usersCount; i++) {
        strncat(out, users[i].login, strlen(users[i].login));
        strncat(out, " ", 1);
    }
}

char *SetPrice(int lot, char buf[], int userNumber) {
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
        char* winner = users[userNumber].login;
        memset(lots[lot].winner, 0, BUF_SIZE);
        strncpy(lots[lot].winner, winner, strlen(winner));
        return "All right!\n";
    }

}

void SentErrServer(char *s) //error handling
{
    perror(s);
    EndTrade();
    exit(1);
}

void NewLot(char name[], char price[], int userNumber) {
    int newPrice = -1;
    newPrice = atoi(price);
    if (newPrice <= 0 || strlen(name) <= 0) {
        SendToClient("Invalid data \n", users[userNumber].cli_addr, users[userNumber].clilen);
        return;
    }
    strcpy(lots[lotCount + 1].lotName, name);
    lots[lotCount + 1].price = newPrice;
    strcpy(lots[lotCount + 1].winner, users[userNumber].login);
    lotCount++;
}

void SendResults() {

    for (int i = 0; i <= usersCount; i++) {
        char result[BUF_SIZE] = "";
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
        SendToClient(result, users[i].cli_addr, users[i].clilen);
    }
}

void EndTrade() {
    shutdown(servSocket, 2);
    close(servSocket);
    exit(1);
}

int main(void) {

    users = (char*) malloc(sizeof (char));
    if (users == NULL) {
        printf("Cant create struct for users");
        EndTrade();
        exit(1);
    }

    lots = (char*) malloc(sizeof (char));
    if (lots == NULL) {
        printf("Cant create struct for lots");
        EndTrade();
        exit(1);
    }

    printf("Server trade is working...\n");

    //Initialization
    struct sockaddr_in local;
    int rc;

    //fill local
    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);

    //make socket
    servSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (servSocket < 0)
        SentErrServer("Socket call failed");

    //attach port
    rc = bind(servSocket, (struct sockaddr *) &local, sizeof (local));
    if (rc < 0)
        SentErrServer("Bind call failure");



    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    //thread for server
    pthread_t client_thread;
    pthread_create(&client_thread, &threadAttr, ClientHandler, (void*) NULL);

    ServerHandler();
    return 0;
}
