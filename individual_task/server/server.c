#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include "pthread.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>
#define POOLN 20

struct User{
    char user_id;
    int sock;
    pthread_t thread_id;
    pthread_t event_on[POOLN];
    char event_on_id[POOLN];
    int kill;

};
struct Event{
    char event_name[10];
    int repeat;
    int interval;
    int event_id;
    pthread_t event_newid[1000];
    int event_launched;
    int event_active;
    int event_stop;
    int delayed;
    int counter;
    int rcounter;
};
struct All_events{
    int event_id;
    int event_launched;
};
char buf[ 1024 ];
int mynumb=0;
int ev_newid = 1;
int connections[POOLN];
struct User user_pool[POOLN];
struct Event event_pool[POOLN];

pthread_t thread;
pthread_t check_inc[POOLN];
pthread_t ev_threads[1000];

void *connectionthread(void *somelist);
void *serverthread(void *somelist);
void *timerprocess(void *somelist);
int getfreeslot(struct Event *whatever);
void *checkcom(void *somelist);
void *eventprocess(void *somelist);

int main( void )
{
    struct sockaddr_in local,text;
    int s,stream_sock;
    int s1,s2;
    int rc,rc2;
    int paramlist[10];

    local.sin_family = AF_INET;
    local.sin_port = htons( 7501 );
    local.sin_addr.s_addr = htonl( INADDR_ANY );
    s = socket( AF_INET, SOCK_STREAM, 0 );
    if ( s < 0 )
    {
        perror( "socket call failed" );
        exit( 1 );
    }
    stream_sock = socket( AF_INET, SOCK_STREAM, 0);
    if (stream_sock < 0)
    {
        perror( "socket call failed" );
        exit( 1 );
    }
    rc = bind( s, ( struct sockaddr * )&local, sizeof( local ) );
    if ( rc < 0 )
    {
        perror( "subscribe call failure" );
        exit( 1 );
    }
    text.sin_family = AF_INET;
    text.sin_port = htons( 7502 );
    text.sin_addr.s_addr = htonl( INADDR_ANY );
    rc2 = bind( stream_sock, ( struct sockaddr * )&text, sizeof( local ) );
    if ( rc < 0 )
    {
        perror( "subscribe call failure" );
        exit( 1 );
    }
    rc = listen( s, 5 );
    if ( rc )
    {
        perror( "listen call failed" );
        exit( 1 );
    }
    rc2 = listen(stream_sock, 5);
    if ( rc2 )
    {
        perror( "listen call failed" );
        exit( 1 );
    }
    pthread_create(&thread,NULL,serverthread,(void*)paramlist);
    int i =1;
    while(1){
        s1=accept(s,NULL,NULL);
        if ( s1 < 0 )
        {
            perror( "accept call failed" );
            exit( 1 );
        }
        s2=accept(stream_sock,NULL,NULL);
        if ( s2 < 0 )
        {
            perror( "accept call failed" );
            exit( 1 );
        }
        paramlist[0]=s1;
        paramlist[2]=s2;
        paramlist[1]=i;
        pthread_create(&(user_pool[i].thread_id),NULL,connectionthread,(void *)paramlist);
        connections[i]=i;
        i++;
    }

    exit( 0 );
}
void *serverthread(void *somelist) {
    pthread_detach(pthread_self());
    printf("Server started. Waiting for the clients...\nEnter any command:\n1)new - create event\n2)add - single or delayed event\n3)events - list of events\n4)start - start delayed event\n5)rem - remove event\n6)show - list users\n7)subscribe - subscribe user to event\n8)unsubscribe - unsubscribe user from event\n9)kill - disconnect user\n10)exit - terminate system\n11)help - command list\n");
    while(1){

        char s[20];
        fgets(s, sizeof(s), stdin);
        if(s[0]=='h' & s[1]=='e' & s[2]=='l' & s[3]=='p')
        {
          
            printf("Command list:\n1)new - create event\n2)add - single or delayed event\n3)events - list of events\n4)start - start delayed event\n5)rem - remove event\n6)show - list users\n7)subscribe - subscribe user to event\n8)unsubscribe - unsubscribe user from event\n9)kill - disconnect user\n10)exit - terminate system\n11)help - command list\n");
            continue;
        }
        if(s[0]=='e' & s[1]=='x' & s[2]=='i' & s[3]=='t')
        {
            int i, test;
            for (i = 0; i < POOLN; i++) {
                if (user_pool[i].user_id != NULL) {
                    test = send(user_pool[i].sock, "kill", 30, 0);
                    close(user_pool[i].sock);
                }
            }
            exit(0);
        }
        if(s[0]=='k' & s[1]=='i' & s[2]=='l' & s[3]=='l')
        {
            int m, mm;
            mm = 0;

            for (m = 0; m < POOLN; m++) {
                if (user_pool[m].user_id != NULL) {
                    mm = 1;
                    break;
                }
            }

            if (!mm) {
                printf("There is no users to kill.\n");
                continue;
            }

            char k[4]="";
            k[0]=s[5],k[1]=s[6],k[2]=s[7];
            if (strcmp(k,"all")==0){
                int i,test1,test,number;
                number = 0;
                for(i=0;i<POOLN;i++){
                    if(user_pool[i].user_id!=NULL) {
                        test = send(user_pool[i].sock, "kill", 30, 0);
                        pthread_cancel(user_pool[i].thread_id);
                        close(user_pool[i].sock);
                        check_inc[i]=NULL;
                        user_pool[i].sock=NULL;
                        user_pool[i].thread_id=NULL;
                        user_pool[i].user_id=NULL;
                        user_pool[i].user_id=NULL;
                        test1=1;
                        number++;
                    }
                }
                if (test1 != 1) {
                    printf("Users not found!\n");
                }
                else {
                    printf("%d users was removed succesfully!\n",number);
                }
            }
            else {
                int tmp = atoi(k);
                int test2 = 0;
                if (tmp != 0) {
                    printf("Trying to kill user ");
                    printf(" %d\n", tmp);
                    int ii, test;
                    for (ii = 0; ii < POOLN; ii++) {
                        if (user_pool[ii].user_id == tmp) {
                            test = send(user_pool[ii].sock, "kill", 30, 0);
                            pthread_cancel(user_pool[ii].thread_id);
                            close(user_pool[ii].sock);
                            check_inc[ii] = NULL;
                            user_pool[ii].sock = NULL;
                            user_pool[ii].thread_id = NULL;
                            user_pool[ii].user_id = NULL;
                            test2 = 1;
                        }
                    }

                    if (test2 != 1) {
                        printf("User not found!\n");
                    } else {
                        printf("User was removed succesfully!\n");
                    }
                } else {
                    printf("Wrong command! No user id entered!\n");
                }
            }
        }
        else if(s[0]=='s' & s[1]=='h' & s[2]=='o' & s[3]=='w')
        {
            int i,z;
            z=0;
            for(i=0;i<POOLN;i++) {
                if(user_pool[i].user_id!=NULL) {
                    z=1;
                    printf("\nUser %d exists!",user_pool[i].user_id);
                    struct sockaddr_in new_socket;
                    unsigned int len = sizeof(new_socket);
                    getpeername(user_pool[i].sock,(struct sockaddr*)&new_socket,&len);
                    printf(" IP: %s ",inet_ntoa(new_socket.sin_addr));
                    int k;
                    for(k=0;k<POOLN;k++) {
                        if(event_pool[k].event_id==NULL) {

                            continue;
                        }

                        int g;
                        for(g=0;g<POOLN;g++) {
                            if(event_pool[k].event_id==user_pool[i].event_on_id[g]) {
                                printf("Event: %s ",event_pool[k].event_name);
                            }
                        }

                    }
                }

            }
            if(!z){
                printf("There is no users.\n");
            }
        }
        else if(s[0]=='n' & s[1]=='e' & s[2]=='w')
        {
            printf("Please enter the name of the event(10 chars or less)\n");
            char pr_name[10];
            fgets(pr_name, sizeof(pr_name), stdin);
            if (pr_name[0]=='\n') {
                printf("Name enter error.\n");
                continue;
            }
            printf("Please enter the interval in seconds, when the process must occur\n");
            char interval[7];
            fgets(interval, sizeof(interval), stdin);
            if (interval[0]=='\n') {
                printf("Interval enter error.\n");
                continue;
            }

            int interv = atoi(interval);
            if (interv == 0) {
                printf("Interval must be a digit and not a zero.\n");
                continue;
            }
            printf("Please enter the number of times you want to repeat that event('0' for infinity)\n");

            fflush(stdout);
            char repeat[6];
            fgets(repeat,sizeof(repeat),stdin);
            if (repeat[0]=='\n') {
                printf("Repeat enter error.\n");
                continue;
            }
            int rep = atoi(repeat);

            int i=0;
            while(pr_name[i]!='\0') {
                if(pr_name[i]=='\n') {
                    pr_name[i]='\0';
                }
                i++;
            }

            int iii;
            int z=0;
            for(iii=0;iii<POOLN;iii++)
            {
                if(event_pool[iii].event_id==NULL)
                {
                    event_pool[iii].event_active = 1;
                    event_pool[iii].event_id = ev_newid;
                    event_pool[iii].interval = interv;
                    event_pool[iii].repeat = rep;
                    event_pool[iii].counter = 0;
                    strcpy(event_pool[iii].event_name,pr_name);
                    break;
                }

            }

            int paramlist[1];
            paramlist[0]=iii;

            printf("Process '%s' has been launched!\n",event_pool[iii].event_name);
            fflush(stdout);
            pthread_create(&(ev_threads[iii]),NULL,eventprocess,(void *)paramlist);
            mynumb++;
            ev_newid++;
        }
        else if(s[0]=='a' & s[1]=='d' & s[2]=='d' & s[3]=='1')
        {
            printf("Please enter the name of the event(10 chars or less)\n");
            char pr_name[10];
            fgets(pr_name, sizeof(pr_name), stdin);
            if (pr_name[0]=='\n') {
                printf("Name enter error.\n");
                continue;
            }
            printf("Please enter how soon to start the process(in seconds) or enter 'd' to make it delayed\n");
            char interval[7];
            fgets(interval, sizeof(interval), stdin);
            if (interval[0]=='\n') {
                printf("Empty field entered.\n");
                continue;
            }

            int interv,res;

            int ii=0;
            while(interval[ii]!='\0') {
                if(interval[ii]=='\n') {
                    interval[ii]='\0';
                }
                ii++;
            }
            ii=0;
            while(pr_name[ii]!='\0') {
                if(pr_name[ii]=='\n') {
                    pr_name[ii]='\0';
                }
                ii++;
            }

            int iii;
            int z=0;
            for(iii=0;iii<POOLN;iii++)
            {
                if(event_pool[iii].event_id==NULL)
                {
                    if (strcmp(interval, "d") == 0) {
                        interv = 0;
                        event_pool[iii].event_active = 0;
                        event_pool[iii].delayed = 1;

                    } else {
                        res = atoi(interval);
                        if (res == 0) {
                            printf("Interval enter error.\n");
                            continue;
                        } else {
                            interv = res;
                        }
                        event_pool[iii].event_active = 0;
                        event_pool[iii].delayed = 1;
                    }
                    event_pool[iii].event_id = ev_newid;
                    event_pool[iii].interval = interv;
                    event_pool[iii].repeat = 1;
                    event_pool[iii].counter = 0;
                    strcpy(event_pool[iii].event_name,pr_name);
                    break;
                }
            }

            int paramlist[1];
            paramlist[0]=iii;

            printf("Delayed process '%s' has been added!\n",event_pool[iii].event_name);
            fflush(stdout);
            pthread_create(&(ev_threads[iii]),NULL,eventprocess,(void *)paramlist);
            mynumb++;
            ev_newid++;
        }
        else if(s[0]=='s' & s[1]=='t' & s[2]=='a' & s[3]=='r' & s[4]=='t')
        {
            printf("Enter the event's name to start from the list:\n");
            int iii;
            int z=0;
            for(iii=0;iii<POOLN;iii++)
            {
                if(event_pool[iii].event_id!=NULL & event_pool[iii].delayed == 1)
                {
                    z=1;
                    printf("Delayed event %s.\n",event_pool[iii].event_name);
                }

            }
            if(!z){
                printf("There is no delayed events.\n");
                continue;
            }

            char name[10];
            fgets(name, sizeof(name), stdin);
            if (name[0]=='\n') {
                printf("Name enter error.\n");
                continue;
            }


            int ii=0;
            while(name[ii]!='\0') {
                if(name[ii]=='\n') {
                    name[ii]='\0';
                }
                ii++;
            }
            int zz=0;
            for(iii=0;iii<POOLN;iii++) {
                if((strcmp((event_pool[iii].event_name),name)==0) & event_pool[iii].delayed==1) {
                    event_pool[iii].event_active = 1;
                    zz=1;
                    printf("Delayed event %s has been started!\n",event_pool[iii].event_name);
                    continue;
                }
            }
            if (!zz) {
                printf("Not found.\n");
                continue;
            }
        }
        else if(s[0]=='e' & s[1]=='v' & s[2]=='e' & s[3]=='n' & s[4]=='t'& s[5]=='s')
        {
            int ii;
            int z=0;
            for(ii=0;ii<POOLN;ii++)
            {
                if(event_pool[ii].event_id!=NULL)
                {
                    z=1;

                    printf("Event %d exists! Event name %s.\n",event_pool[ii].event_id,event_pool[ii].event_name);
                    if (event_pool[ii].delayed==1) {

                        printf("Delayed event. ");
                        if (event_pool[ii].interval != NULL) {
                            printf("Delayed for %d seconds.\n",event_pool[ii].interval);
                        } else {
                            printf("Starts just by user.\n");
                        }

                    }
                    else {
                        if(event_pool[ii].repeat!=0){
                            printf("Interval: %d. Number of repeats: %d. State of repeats: %d\n",event_pool[ii].interval,event_pool[ii].repeat,event_pool[ii].counter);
                        } else printf("Interval: %d. Number of repeats: infinity. State of repeats: %d\n",event_pool[ii].interval,event_pool[ii].counter);
                    }
                }
            }
            if(!z){
                printf("There is no events.\n");
            }
        }
        else if(s[0]=='b' & s[1]=='i' & s[2]=='n' & s[3]=='d')
        {
            int m,mm,ev;
            mm = 0;
            ev = 0;
            for (m = 0; m< POOLN; m++) {
                if (event_pool[m].event_id != NULL) {
                    ev = 1;
                }
                if (user_pool[m].user_id != NULL) {
                    mm = 1;
                }

            }
            if (!mm & !ev){
                printf("There is no users and no events to subscribe.\n");
                continue;
            }
            if (!mm){
                printf("There is no users to subscribe.\n");
                continue;
            }
            if (!ev){
                printf("There is no events to subscribe.\n");
                continue;
            }
            printf("Please enter user id\n");
            char us_id[10];
            fgets(us_id, sizeof(us_id), stdin);
            if (us_id[0]=='\n') {
                printf("User id enter error.\n");
                continue;
            }
            int user_id=atoi(us_id);
            if (user_id == 0){
                printf("User id enter error. Id must be an integer and not a zero.\n");
                continue;
            }
            int ii,marker1;
            marker1=0;
            for (ii = 0; ii < POOLN; ii++) {
                if (strcmp(&(user_pool[ii].user_id), us_id) == 0 & user_pool[ii].user_id != NULL) {

                    marker1 = 1;

                    break;
                }
            }
            if (!marker1){
                printf("There is no user with id %d.\n",user_id);
                continue;
            }

            printf("Please enter event name\n");
            char ev_id[10];
            fgets(ev_id,sizeof(ev_id), stdin);
            if (ev_id[0]=='\n') {
                printf("Event name enter error.\n");
                continue;
            }

            ii=0;
            while(ev_id[ii]!='\0') {
                if(ev_id[ii]=='\n') {
                    ev_id[ii]='\0';
                }
                ii++;
            }

            int marker = 0;
            int identif;
            if (ev_id[0] != '\0') {
                int event_id;
                for (ii = 0; ii < POOLN; ii++) {

                    if (strcmp((event_pool[ii].event_name), ev_id) == 0) {
                        identif = ii;
                        marker = 1;

                        break;
                    }
                }

                int paramlist[2];
                paramlist[0]=identif;
                int z=0;
                int zz;
                int k,newid;
                if (marker){
                    for (zz = 0; zz < POOLN; zz++) {
                        if (user_pool[zz].user_id == user_id) {


                            for (k = 0; k < POOLN; k++) {
                                if (user_pool[zz].event_on[k] == NULL) {

                                    paramlist[1]=user_pool[zz].sock;
                                    for (newid = 0; newid < 1000; newid++){
                                        if (event_pool[identif].event_newid[newid]==NULL) break;

                                    }
                                    event_pool[identif].event_stop = 0;
                                    pthread_create(&(event_pool[identif].event_newid[newid]),NULL,timerprocess,(void *)paramlist);
                                    user_pool[zz].event_on[k] = event_pool[identif].event_newid[newid];
                                    user_pool[zz].event_on_id[k] = event_pool[identif].event_id;
                                    char my_mess[30];
                                    int test;
                                    sprintf(my_mess, "subscribed to the event %s.", event_pool[identif].event_name);
                                    test = send(user_pool[zz].sock, my_mess, 30, 0);

                                    printf("subscribe successful!\n");
                                    z = 1;
                                    break;

                                }
                            }
                        }
                    }
                }
                if (!z) {
                    printf("Error. Not found!\n");
                }
            }
            else{
                printf("Error. No name entered!\n");
            }
        }
        else if(s[0]=='u' & s[1]=='n' & s[2]=='b' & s[3]=='i' & s[4]=='n' & s[5]=='d')
        {
            int m,mm,ev;
            mm = 0;
            ev = 0;
            for (m = 0; m< POOLN; m++) {
                if (event_pool[m].event_id != NULL) {
                    ev = 1;
                }
                if (user_pool[m].user_id != NULL) {
                    mm = 1;
                }

            }
            if (!mm & !ev){
                printf("There is no users and no events to unsubscribe.\n");
                continue;
            }
            if (!mm){
                printf("There is no users to unsubscribe.\n");
                continue;
            }
            if (!ev){
                printf("There is no events to unsubscribe.\n");
                continue;
            }
            printf("Please enter the id of the user:\n");
            char us[10];

            fgets(us,sizeof(us),stdin);
            if (us[0]=='\n') {
                printf("ID enter error.\n");
                continue;
            }

            char what_u_id=atoi(us);
            if (what_u_id == 0){
                printf("User id enter error. Id must be an integer and not a zero.\n");
                continue;
            }
            int ii,marker1;
            marker1=0;
            for (ii = 0; ii < POOLN; ii++) {
                if (strcmp(&(user_pool[ii].user_id), &what_u_id) == 0 & user_pool[ii].user_id != NULL) {

                    marker1 = 1;

                    break;
                }
            }
            if (!marker1){
                printf("There is no user with id %d.\n",what_u_id);

                continue;
            }

            printf("Please enter the name of the event:\n");
            char unb[10];
            fgets(unb,sizeof(unb),stdin);

            if (unb[0]=='\n') {
                printf("Event name enter error.\n");
                continue;
            }
            ii=0;
            while(unb[ii]!='\0') {
                if(unb[ii]=='\n') {
                    unb[ii]='\0';
                }
                ii++;
            }
            int marker = 0;
            int k,z,cc;

            if (unb[0] != '\0') {
                for (ii = 0; ii < POOLN; ii++) {
                    if (strcmp((event_pool[ii].event_name), unb) == 0) {
                        for(k=0;k<POOLN;k++){

                            if (user_pool[k].user_id == what_u_id){
                                for(z=0;z<POOLN;z++) {
                                    if (user_pool[k].event_on_id[z] == event_pool[ii].event_id ) {

                                        pthread_cancel(user_pool[k].event_on[z]);

                                        for (cc=0;cc<1000;cc++) {
                                            if (event_pool[ii].event_newid[cc] == user_pool[k].event_on[z]) {
                                                event_pool[ii].event_newid[cc] = NULL;
                                                continue;
                                            }
                                        }
                                        char my_mess[30];
                                        int test;
                                        sprintf(my_mess, "Unsubscribed from the event %s.", event_pool[ii].event_name);
                                        test = send(user_pool[k].sock, my_mess, 30, 0);
                                        user_pool[k].event_on[z] = NULL;
                                        user_pool[k].event_on_id[z] = NULL;
                                        marker = 1;
                                        printf("Event has been detached!\n");
                                        break;
                                    }
                                }
                            }

                        }
                    }
                }
                if (!marker) {
                    printf("Error. Not found!\n");
                }
            }
            else {
                printf("Error. No name entered!\n");
            }
        }
        else if(s[0]=='r' & s[1]=='e' & s[2]=='m')
        {
            printf("Please enter the name of the event you wish to delete:\n");
            char remedy[10];
            fgets(remedy,sizeof(remedy),stdin);

            int ii=0;
            while(remedy[ii]!='\0') {
                if(remedy[ii]=='\n') {
                    remedy[ii]='\0';
                }
                ii++;
            }
            if (remedy[0]!='\0') {
                int newc;
                for(ii=0;ii<POOLN;ii++) {
                    if(strcmp((event_pool[ii].event_name),remedy)==0) {
                        newc=event_pool[ii].event_id;
                        break;
                    }
                }
                int z=0;
                int newid;
                for(ii=0;ii<POOLN;ii++)
                {
                    if(event_pool[ii].event_id==newc & event_pool[ii].event_id!=NULL)
                    {
                        for (newid = 0; newid < 1000; newid++) {
                            if (event_pool[ii].event_newid[newid]!= NULL){
                                pthread_cancel(event_pool[ii].event_newid[newid]);
                                event_pool[ii].event_newid[newid] = NULL;
                            }
                        }
                        pthread_cancel(ev_threads[ii]);
                        event_pool[ii].event_id = NULL;
                        event_pool[ii].event_id = NULL;
                        event_pool[ii].counter = NULL;
                        event_pool[ii].interval = NULL;
                        event_pool[ii].delayed = 0;
                        event_pool[ii].repeat = NULL;
                        printf("Terminated!\n");
                        z = 1;
                    }
                }
                if (!z){
                    printf("Error. Not found!\n");
                }
            }
            else{
                printf("Error. No name entered!\n");
            }
        }
        else {
            printf("Unknown command %sType 'help' to see the command list.\n",s);
        }
        int i;
        while(i<20){
            s[i]='\0';
            i++;
        }
    }
    return(NULL);
}
void *connectionthread(void *somelist) {
    int* paramlist=(int*)somelist;
    int s1,s2,test,i;
    s1=paramlist[0];
    s2=paramlist[2];
    i=paramlist[1];
    user_pool[i].user_id=i;
    user_pool[i].sock=s2;  //s1
    struct sockaddr_in new_socket;
    unsigned int len = sizeof(new_socket);
    getpeername(user_pool[i].sock,(struct sockaddr*)&new_socket,&len);
    printf("New user! His ID - %d. IP: %s \n",user_pool[i].user_id,inet_ntoa(new_socket.sin_addr));

    char buffer[1024];

    pthread_create(&(check_inc[i]),NULL,checkcom,(void *)paramlist);

}
void *eventprocess(void *somelist) {
    int* paramlist=(int*)somelist;
    int evid = paramlist[0];
    int interv = event_pool[evid].interval;
    if (event_pool[evid].repeat == 0){
        while (1){
            event_pool[evid].counter ++;
            sleep(interv);
        }
    }

    if (event_pool[evid].delayed != 1){
        while (event_pool[evid].counter != event_pool[evid].repeat ){
            event_pool[evid].counter ++;
            sleep(interv);
        }
    } else {
        while (1){
            if (event_pool[evid].interval!=NULL) {
                sleep(event_pool[evid].interval);
                event_pool[evid].event_active = 1;
                sleep(1);
                break;
            }

            if (event_pool[evid].event_active == 1){
                event_pool[evid].counter ++;
                sleep(1);

                break;
            }
            usleep(1);

        }
    }
    event_pool[evid].event_id=NULL;
    event_pool[evid].counter=NULL;
    event_pool[evid].interval=NULL;
    event_pool[evid].delayed = 0;
    event_pool[evid].repeat=NULL;
    printf("Event %s has been terminated!\n",event_pool[evid].event_name);
    int tt;
    for (tt=0; tt<10;tt++){
        event_pool[evid].event_name[tt]='\0';
    }
}
void *timerprocess(void *somelist) {
    int* paramlist=(int*)somelist;
    int id = paramlist[0];

    int s2 = paramlist[1];
  
    int ii,mm;
    mm=0;

    int test,realcounter,c2;

    c2 = event_pool[id].counter + 1;

    if (!(event_pool[id].delayed==1))
    {
        if(event_pool[id].repeat==0) {
            while (1){
                if (event_pool[id].counter == c2){
                    char my_mess[30];
                    sprintf(my_mess, "Event '%s' happened!", event_pool[id].event_name);
                    test = send(s2, my_mess, 30, 0);
                    break;
                }

            }
            while(1) {
                sleep(event_pool[id].interval);
                event_pool[id].counter = mm;
                char my_mess[30];
                sprintf(my_mess, "Event '%s' happened!", event_pool[id].event_name);
                test = send(s2, my_mess, 30, 0);

                mm++;
            }
        }
        else {
            realcounter = event_pool[id].repeat - c2;
            while (1){
                if (event_pool[id].counter == c2){
                    char my_mess[30];
                    sprintf(my_mess, "Event '%s' happened!", event_pool[id].event_name);
                    test = send(s2, my_mess, 30, 0);
                    break;
                }

            }
            for( ii=0;ii<realcounter;ii++)
            {
                sleep(event_pool[id].interval);
                char my_mess[30];
                sprintf(my_mess, "Event '%s' happened!", event_pool[id].event_name);
                test = send(s2, my_mess, 30, 0);
                if (test<=0)
                {
                    break;
                }
            }
        }
    }
    else {
        while (1) {

            if (event_pool[id].event_active == 1) {
                char my_mess[30];
                sprintf(my_mess, "D.event '%s' happened!", event_pool[id].event_name);
                test = send(s2, my_mess, 30, 0);
                if (test <= 0) {
                    break;
                }
                break;
            }

            usleep(1);
        }

    }

    fflush(stdout);

    return(NULL);
}

void *checkcom(void *somelist)
{
    int* paramlist=(int*)somelist;
    int s1,test,i;
    char s[20];
    s1=paramlist[0];
    i=paramlist[1];

    while(1)
    {
        int rc = recv( s1, s, 20, 0 ) ;
        if( rc <= 0 )
        {
            close(s1);
            pthread_exit(NULL);
            printf("thread closed for user %d\n",(i+1));
        }


        if(s[0]=='d' & s[1]=='i' & s[2]=='s' & s[3]=='c' & s[4]=='o' & s[5]=='n' & s[6]=='n'& s[7]=='e'& s[8]=='c'& s[9]=='t')
        {
            pthread_cancel(user_pool[i].thread_id);
            close(user_pool[i].sock);
            check_inc[i]=NULL;
            user_pool[i].sock=NULL;
            user_pool[i].thread_id=NULL;


            int ii,xx,bb;
            for(ii=0;ii<0;ii++){
                for(xx=0;xx<0;xx++){
                    for(bb=0;bb<0;bb++){
                        if (event_pool[ii].event_newid[xx] == user_pool[i].event_on[bb]) {
                            event_pool[ii].event_newid[xx] == NULL;
                        }
                    }
                }
            }
            for(ii=0;ii<0;ii++){
                pthread_cancel(user_pool[i].event_on[ii]);
                user_pool[i].event_on[ii] = NULL;
            }
            printf("User %d is disconnected.\n",user_pool[i].user_id);
            user_pool[i].user_id=NULL;
            return(NULL);
        }
        else if(s[0]=='s' & s[1]=='h' & s[2]=='o' & s[3]=='w')
        {


            int n=0;
            int k;
            for(k=0;k<POOLN;k++)
            {
                if(event_pool[k].event_id!=NULL)
                {
                    n++;
                }
            }
            char b1[10];
            sprintf(b1,"%d",n);
            rc = send( s1, b1, 10, 0 );
            if(rc <= 0)
            {
                perror( "Step1" ) ;
                exit( 1 );
            }
            int g,zzz,m;
            zzz = 0;
            char b[100];
            char b2[200];
            for(k=0;k<POOLN;k++) {
                zzz = 0;

                if (event_pool[k].event_id != NULL) {
                    for (g = 0; g < POOLN; g++) {


                        if (event_pool[k].event_id == user_pool[i].event_on_id[g]) {

                            sprintf(b, "Event %s exists! SUBSCRIBED EVENT.\n", event_pool[k].event_name);
                            zzz = 1;
                            rc = send(s1, b, 100, 0);

                            if (rc <= 0) {
                                perror("Step1");
                                exit(1);
                            }
                        }
                    }

                    if (!zzz){
                        sprintf(b, "Event %s exists!\n", event_pool[k].event_name);
                        rc = send(s1, b, 100, 0);
                        if (rc <= 0) {
                            perror("Step1");
                            exit(1);
                        }
                    }
                    if (event_pool[k].delayed == 1) {

                        sprintf(b2, "Delayed event. ");
                        if (event_pool[k].interval != NULL) {
                            char tmp[20];
                            sprintf(tmp,"Delayed for %d seconds.\n",event_pool[k].interval);

                            strcat(b2, tmp);
                        } else {
                            char tmp[20];
                            sprintf(tmp,"Starts just by user.\n",event_pool[k].interval);
                            strcat(b2, tmp);
                        }

                    } else {
                        if (event_pool[k].repeat != 0) {
                            sprintf(b2,"Interval: %d. Number of repeats: %d. State of repeats: %d\n", event_pool[k].interval, event_pool[k].repeat, event_pool[k].counter);
                        } else sprintf(b2,"Interval: %d. Number of repeats: infinity. State of repeats: %d\n", event_pool[k].interval, event_pool[k].counter);

                    }
                    rc = send(s1, b2, 200, 0);
                    if (rc <= 0) {
                        perror("Step1");
                        exit(1);
                    }
                }
            }
        }
        else if(s[0]=='r' & s[1]=='e' & s[2]=='m')
        {

            char remedy[10];
            rc = recv( s1, remedy, 10, 0 );
            if(rc <= 0)
            {
                perror( "Remove name is not recieved" ) ;
                exit( 1 );
            }
            int ii=0;
            while(remedy[ii]!='\0') {
                if(remedy[ii]=='\n') {
                    remedy[ii]='\0';
                }
                ii++;
            }
            char code[2];
            if (remedy[0]!='\0') {
                int newc;
                for(ii=0;ii<POOLN;ii++) {
                    if(strcmp((event_pool[ii].event_name),remedy)==0) {
                        newc=event_pool[ii].event_id;
                        break;
                    }
                }
                int z=0;
                int newid;

                for(ii=0;ii<POOLN;ii++)
                {
                    if(event_pool[ii].event_id==newc & event_pool[ii].event_id!=NULL)
                    {
                        for (newid = 0; newid < 1000; newid++) {
                            if (event_pool[ii].event_newid[newid]!= NULL){
                                pthread_cancel(event_pool[ii].event_newid[newid]);
                                event_pool[ii].event_newid[newid] = NULL;
                            }
                        }
                        pthread_cancel(ev_threads[ii]);
                        event_pool[ii].event_id = NULL;
                        event_pool[ii].event_id = NULL;
                        event_pool[ii].counter = NULL;
                        event_pool[ii].interval = NULL;
                        event_pool[ii].delayed = 0;
                        event_pool[ii].repeat = NULL;

                        sprintf(code,"01");
                        rc = send(s1, code, 2, 0);
                        if (rc <= 0) {
                            perror("Step1");
                            exit(1);
                        }
                        z = 1;
                        printf("Process %s terminated by user %d!\n",event_pool[ii].event_name,user_pool[i].user_id);
                    }
                }
                if (!z){
                    sprintf(code,"00");
                    rc = send(s1, code, 2, 0);
                    if (rc <= 0) {
                        perror("Step1");
                        exit(1);
                    }
                }
            }
        }
        else if(s[0]=='n' & s[1]=='e'  & s[2]=='w')
        {

            char pr_name[10];
            rc = recv( s1, pr_name, 10, 0 );
            if(rc <= 0)
            {
                perror( "Name is not recieved" ) ;
                exit( 1 );
            }
            char interval[10];
            rc = recv( s1, interval, 10, 0 );
            if(rc <= 0)
            {
                perror( "Interval is not recieved" ) ;
                exit( 1 );
            }
            int interv = atoi(interval);

            char repeat[10];
            rc = recv( s1, repeat, 10, 0 );
            if(rc <= 0)
            {
                perror( "Repeat is not recieved" ) ;
                exit( 1 );
            }
            int rep = atoi(repeat);
            int ii=0;
            while(interval[ii]!='\0') {
                if(interval[ii]=='\n') {
                    interval[ii]='\0';
                }
                ii++;
            }
            int iiii=0;
            while(pr_name[iiii]!='\0') {
                if(pr_name[iiii]=='\n') {
                    pr_name[iiii]='\0';
                }
                iiii++;
            }
            int iii;
            int z=0;
            for(iii=0;iii<POOLN;iii++)
            {
                if(event_pool[iii].event_id==NULL)
                {

                    event_pool[iii].event_active = 1;

                    event_pool[iii].event_id = ev_newid;
                    event_pool[iii].interval = interv;
                    event_pool[iii].repeat = rep;
                    event_pool[iii].counter = 0;
                    strcpy(event_pool[iii].event_name,pr_name);
                    event_pool[iii].delayed = 0;
                    break;
                }
            }

            int paramlist[1];
            paramlist[0]=iii;
            printf("Process '%s' launched by user %d!\n",event_pool[iii].event_name,user_pool[i].user_id);
            char tmp[100];
            sprintf(tmp,"Process '%s' has been launched!\n",event_pool[iii].event_name);
            rc = send( s1, tmp, 100, 0 );
            if(rc <= 0)
            {
                perror( "Success send error" ) ;
                exit( 1 );
            }

            fflush(stdout);
            pthread_create(&(ev_threads[iii]),NULL,eventprocess,(void *)paramlist);
            mynumb++;
            ev_newid++;
        }
        else if(s[0]=='a' & s[1]=='d'  & s[2]=='d'& s[3]=='1')
        {
            char pr_name[10];
            rc = recv( s1, pr_name, 10, 0 );
            if(rc <= 0)
            {
                perror( "Name is not recieved" ) ;
                exit( 1 );
            }
            char interval[10];
            rc = recv( s1, interval, 10, 0 );
            if(rc <= 0)
            {
                perror( "Name is not recieved" ) ;
                exit( 1 );
            }

            int interv,res;

            int ii=0;
            while(interval[ii]!='\0') {
                if(interval[ii]=='\n') {
                    interval[ii]='\0';
                }
                ii++;
            }
            ii=0;
            while(pr_name[ii]!='\0') {
                if(pr_name[ii]=='\n') {
                    pr_name[ii]='\0';
                }
                ii++;
            }

            int iii;
            int z=0;
            for(iii=0;iii<POOLN;iii++)
            {
                if(event_pool[iii].event_id==NULL)
                {
                    if (strcmp(interval, "d") == 0) {
                        interv = 0;
                        event_pool[iii].event_active = 0;
                        event_pool[iii].delayed = 1;

                    } else {
                        res = atoi(interval);
                        if (res == 0) {
                            rc = send( s1, "1", 1, 0 );
                            if(rc <= 0)
                            {
                                perror( "Name is not recieved" ) ;
                                exit( 1 );
                            }

                            continue;
                        } else {
                            interv = res;
                        }
                        event_pool[iii].event_active = 0;
                        event_pool[iii].delayed = 1;
                    }
                    event_pool[iii].event_id = ev_newid;
                    event_pool[iii].interval = interv;
                    event_pool[iii].repeat = 1;
                    event_pool[iii].counter = 0;
                    strcpy(event_pool[iii].event_name,pr_name);
                    break;
                }
            }

            int paramlist[1];
            paramlist[0]=iii;
            rc = send( s1, "0", 1, 0 );

            printf("Delayed process '%s' has been added by user %d!\n",event_pool[iii].event_name,user_pool[i].user_id);
            char tmp[100];
            sprintf(tmp,"Delayed process '%s' has been launched!\n",event_pool[iii].event_name);
            rc = send( s1, tmp, 100, 0 );
            if(rc <= 0)
            {
                perror( "Success send error" ) ;
                exit( 1 );
            }
            fflush(stdout);
            pthread_create(&(ev_threads[iii]),NULL,eventprocess,(void *)paramlist);
            mynumb++;
            ev_newid++;


        }
        else if(s[0]=='s' & s[1]=='t' & s[2]=='a' & s[3]=='r' & s[4]=='t')
        {

            int n = 0;
            int k;
            for (k = 0; k < POOLN; k++) {
                if (event_pool[k].event_id != NULL && event_pool[k].delayed == 1) {
                    n++;
                }
            }
            char b[10];

            sprintf(b, "%d", n);

            rc = send(s1, b, 10, 0);
            if (rc <= 0) {
                perror("Step1");
                exit(1);
            }

            if (n!=0){
                int iii;
                char mes[90];

                for(iii=0;iii<POOLN;iii++)
                {
                    if(event_pool[iii].event_id!=NULL & event_pool[iii].delayed == 1)
                    {
                        sprintf(mes,"Delayed event %s.\n",event_pool[iii].event_name);

                        rc = send( s1, mes, 90, 0 );
                        if(rc <= 0)
                        {
                            perror( "Step1" ) ;
                            exit( 1 );
                        }
                    }
                }

                char name[10];


                rc= recv(s1,name,10,0);
                if(rc <= 0)
                {
                    perror( "Name not recieved" ) ;
                }

                if (strcmp(name,"err1ror1")==0) continue;

                int zz=0;
                for(iii=0;iii<POOLN;iii++) {
                    if((strcmp((event_pool[iii].event_name),name)==0) & event_pool[iii].event_active==0) {
                        event_pool[iii].event_active = 1;
                        zz=1;
                        char tmp[90];
                        sprintf(tmp, "Delayed event '%s' has been started!\n", event_pool[iii].event_name);
                        rc = send(s1, tmp, 90, 0);
                        if (rc <= 0) {
                            perror("Success send error");
                            exit(1);
                        }
                        printf("Delayed event %s has been started by user %d!\n",event_pool[iii].event_name,user_pool[i].user_id);
                        continue;
                    }
                }
                if (!zz) {
                    char tmp[90];
                    sprintf(tmp, "Error. Event not found!\n");
                    rc = send(s1, tmp, 90, 0);
                    if (rc <= 0) {
                        perror("Event not found send error");
                        exit(1);
                    }
                    continue;
                }
            }
        }
        else if(s[0]=='b' & s[1]=='i'  & s[2]=='n' & s[3]=='d')
        {
            char code[3];
            int m,mm,ev;
            mm = 0;
            ev = 0;
            for (m = 0; m< POOLN; m++) {
                if (event_pool[m].event_id != NULL) {
                    ev = 1;
                }
            }
            if (!ev){
                sprintf(code, "11");
                rc = send(s1, code, 2, 0);
                if (rc <= 0) {
                    perror("Step1");
                    exit(1);
                }

                continue;
            } else {

                sprintf(code, "00"); 
                rc = send(s1, code, 2, 0);
                if (rc <= 0) {
                    perror("Step1");
                    exit(1);
                }
            }

            int user_id = user_pool[i].user_id;

            char ev_id[10];

            rc = recv( s1, ev_id, 10, 0 );
            if(rc <= 0)
            {
                perror( "Name is not recieved" ) ;
                exit( 1 );
            }

            if (strcmp(ev_id,"err1rro")==0) continue;

            int ii;
            int marker = 0;
            int identif;
            if (ev_id[0] != '\0') {
                int event_id;
                for (ii = 0; ii < POOLN; ii++) {

                    if (strcmp((event_pool[ii].event_name), ev_id) == 0) {
                        identif = ii;
                        marker = 1;

                        break;
                    }
                }

                int paramlist[2];
                paramlist[0]=identif;
                int z=0;
                int zz;
                int k,newid;
                if (marker){
                    for (zz = 0; zz < POOLN; zz++) {
                        if (user_pool[zz].user_id == user_id) {


                            for (k = 0; k < POOLN; k++) {
                                if (user_pool[zz].event_on[k] == NULL) {

                                    paramlist[1]=user_pool[zz].sock;
                                    for (newid = 0; newid < 1000; newid++){
                                        if (event_pool[identif].event_newid[newid]==NULL) break;
                                    }
                                    event_pool[identif].event_stop = 0;
                                    pthread_create(&(event_pool[identif].event_newid[newid]),NULL,timerprocess,(void *)paramlist);
                                    user_pool[zz].event_on[k] = event_pool[identif].event_newid[newid];
                                    user_pool[zz].event_on_id[k] = event_pool[identif].event_id;
                                    printf("User %d subscribed on event %s.\n",user_pool[zz].user_id,event_pool[identif].event_name);
                                    sprintf(code, "00");
                                    rc = send(s1, code, 2, 0);
                                    if (rc <= 0) {
                                        perror("Step1");
                                        exit(1);
                                    }
                                    z = 1;
                                    break;
                                }
                            }
                        }
                    }
                }
                if (!z) {
                    sprintf(code, "01");
                    rc = send(s1, code, 2, 0);
                    if (rc <= 0) {
                        perror("Step1");
                        exit(1);
                    }
                }
            }
            else{
                sprintf(code, "11");
                rc = send(s1, code, 2, 0);
                if (rc <= 0) {
                    perror("Step1");
                    exit(1);
                }
            }
        }
        else if(s[0]=='u' & s[1]=='n' &  s[2]=='b' & s[3]=='i' & s[4]=='n' & s[5]=='d')
        {
            char code[3];
            int m,mm;
            mm = 0;

            for (m = 0; m < POOLN; m++) {
                if (user_pool[i].event_on_id[m] != NULL) {

                    mm = 1;
                }
            }
            if (!mm) {
                sprintf(code, "11"); 
                rc = send(s1, code, 2, 0);
                if (rc <= 0) {
                    perror("Step1");
                    exit(1);
                }
                continue;
            } else {
                sprintf(code, "00");
                rc = send(s1, code, 2, 0);
                if (rc <= 0) {
                    perror("Step1");
                    exit(1);
                }

            }

            int what_u_id = user_pool[i].user_id;

            char unb[10];
            rc = recv( s1, unb, 10, 0 );
            if(rc <= 0)
            {
                perror( "Name is not recieved" ) ;
                exit( 1 );
            }

            if (strcmp(unb,"err1rro")==0) continue;



            int marker = 0;
            int k,z,cc,ii;

            if (unb[0] != '\0') {
                for (ii = 0; ii < POOLN; ii++) {
                    if (strcmp((event_pool[ii].event_name), unb) == 0) {
                        for(k=0;k<POOLN;k++){
                            if (user_pool[k].user_id == what_u_id){
                                for(z=0;z<POOLN;z++) {
                                    if (user_pool[k].event_on_id[z] == event_pool[ii].event_id ) {

                                        pthread_cancel(user_pool[k].event_on[z]);

                                        for (cc=0;cc<1000;cc++) {
                                            if (event_pool[ii].event_newid[cc] == user_pool[k].event_on[z]) {
                                                event_pool[ii].event_newid[cc] = NULL;
                                                continue;
                                            }
                                        }
					user_pool[k].event_on[z] = NULL;
                                        user_pool[k].event_on_id[z] = NULL;
                                        marker = 1;

                                        printf("User %d unsubscribed from event %s.\n", user_pool[k].user_id, event_pool[ii].event_name);
                                        sprintf(code, "00");
                                        rc = send(s1, code, 2, 0);
                                        if (rc <= 0) {
                                            perror("Step1");
                                            exit(1);
                                        }
                                        break;
                                    }
                                }
                            }

                        }
                    }
                }
                if (!marker) {
                    sprintf(code, "01");
                    rc = send(s1, code, 2, 0);
                    if (rc <= 0) {
                        perror("Step1");
                        exit(1);
                    }
                }
            }
            else {
                sprintf(code, "11");
                rc = send(s1, code, 2, 0);
                if (rc <= 0) {
                    perror("Step1");
                    exit(1);
                }
            }

        }

        int i;
        while(i<20){
            s[i]='\0';
            i++;
        }
    }
}
int getfreeslot(struct Event *whatever)
{
    int i;
    for(i=0;i<sizeof(whatever)/sizeof(int);i++)
    {
        if(&(whatever[i]).event_id==NULL) return i;
    }
}
