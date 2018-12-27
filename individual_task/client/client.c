#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include "pthread.h"

void *recievethread(void *somelist);
void *mainprocess(void *somelist);
pthread_t reciever,proc;
int killpr = 0;
int main( void )
{
   int paramlist[5];
   pthread_create(&(proc),NULL,(void*)mainprocess,(void *)paramlist);
   
   for (;;){
       usleep(1);
       if (killpr == 1) exit(0);
   }
}

void *mainprocess(void *somelist){
     
struct sockaddr_in peer;
int s,sock,sock2,ms,recieve,paramlist[5];
int rc;
char buf[1];
printf("Please enter the ip address of the server\n");
char serv[15]="127.0.0.1";
printf("Please enter the port of the server\n");
char port[6]="7501";
int int_port=atoi(port);
peer.sin_family = AF_INET;
peer.sin_port = htons(int_port);
peer.sin_addr.s_addr = inet_addr( serv );
ms = socket( AF_INET, SOCK_STREAM, 0 );
if(ms < 0)
{
    perror( "error socket" );
    exit( 1 );
}
recieve = socket( AF_INET, SOCK_STREAM, 0 );
if(recieve < 0)
{
    perror( "error socket" );
    exit( 1 );
}
sock = connect( ms,( struct sockaddr * )&peer, sizeof(peer) );
if(sock)
{
    perror( "error connect" );
    exit( 1 );
}
peer.sin_port = htons(7502);
sock2 = connect( recieve,( struct sockaddr * )&peer, sizeof(peer) );
if(sock2)
{
    perror( "error connect" );
    exit( 1 );
}
printf("Successfully connected to server %s \n",serv);
paramlist[0]=recieve;
pthread_create(&(reciever),NULL,(void*)recievethread,(void *)paramlist);
fflush(stdout);
printf("Command list:\n1)new - create event\n2)add - single or delayed event\n3)start - start delayed event\n4)show - events list\n5)rem - remove event\n6)subscribe - subscribe to event\n7)unsubscribe - unsubscribe from event\n8)disconnect - disconnect fom server\n9)help - command list\n");

while(1)
{
    char s[20];
    fgets(s, sizeof(s), stdin);

    if (s[0] == 'h' & s[1] == 'e' & s[2] == 'l' & s[3] == 'p') {
        printf("Command list:\n1)new - create event\n2)add - single or delayed event\n3)start - start delayed event\n4)show - events list\n5)rem - remove event\n6)sub - subscribe to event\n7)unsub - unsubscribe from event\n8)disconnect - disconnect fom server\n9)help - command list\n");
        continue;
    }
    
    if(s[0]=='d' & s[1]=='i' & s[2]=='s' & s[3]=='c' & s[4]=='o' & s[5]=='n' & s[6]=='n'& s[7]=='e'& s[8]=='c'& s[9]=='t')//отключение
    {
        rc = send( ms, "disconnect", 20, 0 );
        if(rc <= 0)
        {
            perror( "error send" ) ;
            exit( 1 );
        }
        printf("Client closed!\n");
        exit(0);
    }
    else if(s[0]=='s' & s[1]=='h' & s[2]=='o' & s[3]=='w')//показать все доступные события
    {
        rc = send( ms, "show", 20, 0 );
        if(rc <= 0)
        {
            perror( "Show error send" ) ;
            exit( 1 );
        }
        
        char tmp[100];
        char tmp2[200];
        char num[10];
        rc= recv(ms,num,10,0);
        if(rc <= 0)
        {
            perror( "Show recieve error" ) ;
            exit( 1 );
        }
        int numb = atoi(num);
        
        if (numb == 0){
                printf("Event list is empty.\n");
                continue;
        }
        int i;
        for(i=0;i<numb;i++) {
            rc = recv(ms,tmp,100,0);
            if(rc <= 0)
            {
                perror( "Show not recieved" ) ;
                exit( 1 );
            }
            printf("%s",tmp);
            rc = recv(ms,tmp2,200,0);
            if(rc <= 0)
            {
                perror( "Show not recieved" ) ;
                exit( 1 );
            }
            printf("%s",tmp2);
        }
        fflush(stdout);

    }
    else if(s[0]=='r' & s[1]=='e' & s[2]=='m')//удаление события
    {
        
        printf("Please enter the name of the event you wish to delete:\n");
        char remedy[10];
        fgets(remedy,sizeof(remedy),stdin);
        if (remedy[0]=='\n'){
            printf("No name entered.\n");
            continue;
        }
        
        rc = send( ms, "rem", 20, 0 );
        if(rc <= 0)
        {
            perror( "error send" ) ;
            exit( 1 );
        }
        
        rc = send( ms, remedy, 10, 0 );
        if(rc <= 0)
        {
            perror( "error send" ) ;
            exit( 1 );
        }
             
        char code[3];
        rc= recv(ms,code,2,0);
        if(rc <= 0)
        {
            perror( "Show not recieved" ) ;
            exit( 1 );
        }
        if (strcmp(code,"01")==0) {
            printf("Terminated!\n");
        }
        if (strcmp(code,"00")==0) {
              printf("Error. Not found!\n"); 
        }
        if (strcmp(code,"11")==0) {
              printf("Error. No name entered!\n"); 
        }
    }
    else if(s[0]=='n' & s[1]=='e'  & s[2]=='w')//Добавление события
    {
        char new[100];
        printf("Please enter the name of the event(10 chars or less)\n");
        char pr_name[10];
        fgets(pr_name, sizeof(pr_name), stdin);
        if (pr_name[0]=='\n') {
                printf("Name enter error.\n");
                continue;
        }
        printf("Please enter the interval in seconds, when the process must occur\n");
         char interval[10];   
        fgets(interval, sizeof(interval), stdin);
        if (interval[0]=='\n') {
                printf("Interval enter error.\n");
                continue;
            }
        char temp1;
        temp1 = atoi(interval);
        if (temp1 == 0) {
                    printf("Interval must be a digit and not a zero.\n");
                    continue;
        }
        printf("Please enter the number of times you want to repeat that event('0' for infinity)\n");
        char repeat[10];
        fgets(repeat, sizeof(repeat), stdin);
        if (repeat[0]=='\n') {
                printf("Repeat enter error.\n");
                continue;
            }
        
        rc = send( ms, "new", 20, 0 );
        if(rc <= 0)
        {
            perror( "error 'new' send" ) ;
            exit( 1 );
        }
        
        rc = send( ms, pr_name, 10, 0 );
        if(rc <= 0)
        {
            perror( "error name send" ) ;
            exit( 1 );
        }

        rc = send(ms, interval, 10, 0 );
        if(rc <= 0)
        {
            perror( " interval error send" ) ;
            exit( 1 );
        }
        rc = send( ms, repeat, 10, 0 );
        if(rc <= 0)
        {
            perror( "repeat error send" ) ;
            exit( 1 );
        }
        rc= recv(ms,new,100,0);
        if(rc <= 0)
        {
            perror( "Show not recieved" ) ;
            exit( 1 );
        }
        printf("%s",new);
        
        fflush(stdout);
    }
    else if(s[0]=='a' & s[1]=='d'  & s[2]=='d')//Добавление события
    {
        char new[100];
        printf("Please enter the name of the event(10 chars or less)\n");
        char pr_name[10];
        fgets(pr_name, sizeof(pr_name), stdin);
        if (pr_name[0]=='\n') {
                printf("Name enter error.\n");
                continue;
        }
        printf("Please enter how soon to start the process(in seconds) or enter 'd' to make it delayed\n");
         char interval[10];   
        fgets(interval, sizeof(interval), stdin);
        if (interval[0]=='\n') {
                printf("Interval empty field enter error.\n");
                continue;
        }
        rc = send( ms, "add1", 20, 0 );
        if(rc <= 0)
        {
            perror( "error 'new' send" ) ;
            exit( 1 );
        }
        
        rc = send( ms, pr_name, 10, 0 );
        if(rc <= 0)
        {
            perror( "error name send" ) ;
            exit( 1 );
        }

        rc = send(ms, interval, 10, 0 );
        if(rc <= 0)
        {
            perror( " interval error send" ) ;
            exit( 1 );
        }
        char err[20];
        rc= recv(ms,err,1,0);
        if(rc <= 0)
        {
            perror( "err code not recieved" ) ;
            exit( 1 );
        }
        if (strcmp(err,"1")==0) {
            printf("Error. You can enter 'd' or time of delay (digit).\n");
            continue;
        }
        
        rc= recv(ms,new,100,0);
        if(rc <= 0)
        {
            perror( "Show not recieved" ) ;
            exit( 1 );
        }
        printf("%s",new);
        
        fflush(stdout);
    }
    else if(s[0]=='s' & s[1]=='u' &  s[2]=='b')//Привязка события
    {
        rc = send( ms, "bind", 20, 0 );
        if(rc <= 0)
        {
            perror( "error send" ) ;
            exit( 1 );
        }
        
        char code[3];
        rc= recv(ms,code,2,0);
        if(rc <= 0)
        {
            perror( "Show not recieved" ) ;
            exit( 1 );
        }
        if (strcmp(code,"11")==0) {
             printf("There is no events to subscribe.\n");
             continue;
        }
        printf("Please enter event name\n");
        
        char name[10];
        fgets(name, sizeof(name), stdin);
        if (name[0]=='\n') {
                printf("Event name enter error.\n");
                rc = send( ms, "err1rro", 10, 0 );
                if(rc <= 0)
                {
                    perror( "name send error" ) ;
                    exit( 1 );
                }
                continue;
            }
        
        int k=0;
        while(name[k]!='\0') {
            if(name[k]=='\n') {
               name[k]='\0';
            }
            k++;
        }
        rc = send( ms, name, 10, 0 );
        if(rc <= 0)
        {
            perror( "name send error" ) ;
            exit( 1 );
        }
        
        
        
        rc= recv(ms,code,2,0);
        if(rc <= 0)
        {
            perror( "Show not recieved" ) ;
            exit( 1 );
        }

        if (strcmp(code,"00")==0) {
            printf("subscribe successful!\n");
            
        }
        if (strcmp(code,"01")==0) {
            printf("Error. Not found!\n");
            
        }
        if (strcmp(code,"11")==0) {\
              printf("Error. No name entered!\n"); 
            
        }
    }
    else if(s[0]=='s' & s[1]=='t' &  s[2]=='a' & s[3]=='r'& s[4]=='t')//Привязка события
    {
        rc = send( ms, "start", 20, 0 );
        if(rc <= 0)
        {
            perror( "error send" ) ;
            exit( 1 );
        }
        char tmp[10];
        rc= recv(ms,tmp,10,0);
        if(rc <= 0)
        {
            perror( "Show not recieved" ) ;
            exit( 1 );
        }
        int numb = atoi(tmp);
        
        
        if (numb!=0){
            char mes[90];
            printf("Enter the event's name to start from the list:\n");
            int i;
            for(i=0;i<numb;i++) {
                rc= recv(ms,mes,90,0);
                if(rc <= 0)
                {
                    perror( "Show not recieved" ) ;
                    exit( 1 );
                }
                printf("%s",mes);
            }
            char name[10];
            fgets(name, sizeof(name), stdin);
            if (name[0]=='\n') {
                    printf("Name enter error.\n");
                    rc = send( ms, "err1ror1", 10, 0 );
                    if (rc <= 0) {
                        perror("Err code send error");
                        exit(1);
                    }
                    continue;
            }
            int k=0;
            while (name[k] != '\0') {
                if (name[k] == '\n') {
                    name[k] = '\0';
                }
                k++;
            }
            rc = send(ms, name, 10, 0);
            if (rc <= 0) {
                perror("error send");
                exit(1);
            }
            char msg[90];
            rc= recv(ms,msg,90,0);
            if(rc <= 0)
                {
                    perror( "Show not recieved" ) ;
                    exit( 1 );
                }
            if (msg[0]=='D'){
            printf("%s",msg);
            } else {
              printf("%s",msg);
            } 
        }
            else {
                printf("There are no delayed events.\n");
            
            }   
    }
    else if(s[0]=='u' & s[1]=='n' &  s[2]=='s' & s[3]=='u' & s[4]=='b')//Отмена подписки
    {
        
        
        rc = send( ms,"unbind", 20, 0 );
        if(rc <= 0)
        {
            perror( "error send" ) ;
            exit( 1 );
        }
        
        char code[3];
        rc= recv(ms,code,2,0);
        if(rc <= 0)
        {
            perror( "Show not recieved" ) ;
            exit( 1 );
        }

        if (strcmp(code,"11")==0) {
             printf("There is no subscribed events.\n");
             continue;
        }
        
        printf("Please enter event name\n");
        
        char name[10];
        fgets(name, sizeof(name), stdin);
        if (name[0]=='\n') {
                printf("Event name enter error.\n");
                rc = send( ms, "err1rro", 10, 0 );
                if(rc <= 0)
                {
                    perror( "name send error" ) ;
                    exit( 1 );
                }
                continue;
            }
        
        int k=0;
        while(name[k]!='\0') {
            if(name[k]=='\n') {
               name[k]='\0';
            }
            k++;
        }
        rc = send( ms, name, 10, 0 );
        if(rc <= 0)
        {
            perror( "name send error" ) ;
            exit( 1 );
        }
        rc= recv(ms,code,2,0);
        if(rc <= 0)
        {
            perror( "Show not recieved" ) ;
            exit( 1 );
        }

        if (strcmp(code,"00")==0) {
            printf("Successful unsubscribe!\n");
            
        }
        if (strcmp(code,"01")==0) {
            printf("Error. Not found!\n");
            
        }
        if (strcmp(code,"11")==0) {
              printf("Error. No name entered!\n"); 
        }    
    }
    else {
            printf("Unknown command %sType 'help' to view the command list.\n",s);
        }
    int i;
    while(i<20)
    {
        s[i]="\0";
        i++;
    }
}
}
void *recievethread(void *somelist) {
int* paramlist=(int*)somelist;
int s2,rc;
char s[30];
s2=paramlist[0];
fflush(stdout);
while(1) {
    rc= recv(s2,s,30,0);
    if(s[0]=='k' & s[1]=='i' & s[2]=='l' & s[3]=='l' ) {
       printf("CLIENT TERMINATED.\n");
       
       killpr = 1;
       break; 
        
    }
    if(rc <= 0)
    {
        perror( "Show not recieved" ) ;
        
        exit( 1 );
    }
    printf("%s\n",s);
    fflush(stdout);
}
}
