#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <windows.h>
#include <string.h>
#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace std;

char SERVER_IP[15] = "127.0.0.1";
int  SERVER_PORT   = 8080;
bool working       = true;
char buf[8];
int sock;

int ServerMessageReceive(string &line){
    int rc = 1;
    while(rc != 0){
        rc = recv(sock, buf, 1, 0);
        if(rc <= 0) return 0;
        if(buf[0] == '\n') return 1;
        line = line + buf[0];
    }
}

DWORD WINAPI ProcessClient(LPVOID msg){   
	while(working){
		string msg;
		getline(cin,msg);

        if(msg == "quit"){
            working = false;
            close(sock);
            exit(0);            
        }

        msg += "\n";
        
        if(send(sock, msg.data(), msg.size(), 0) < 0){
            puts("Send error");
            working = false;
            close(sock);
            exit(1);
        }
    }
}

int main(int argc, char *argv[]){
	 WSADATA wsdt;
	 int ws;
	 HANDLE t1;
    int opt, fullCheck;
    struct sockaddr_in server;
    string line;


    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(SERVER_PORT);
	ws = WSAStartup(MAKEWORD(2,2), &wsdt);
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0){
        perror("ERROR Socket");
		WSACleanup();
        return(1);
    }else puts("Success Socket creating");

    if(connect(sock, (struct sockaddr *) &server, sizeof(server)) < 0){
        perror("ERROR Connect");
		WSACleanup();
        return(2);
    }else puts("Success Connect");


    t1 = CreateThread(NULL, 0, ProcessClient, &sock, 0, NULL);
    while(working){
        line = "";
        if(ServerMessageReceive(line) == 1){
            cout << line << endl;
            
            fullCheck = strncmp(line.data(), "\nERROR, Server is full", 15);
            if(fullCheck == 0) working = false;
        }
        else{
            cout << "You have been disconnected" << endl;
            working = false;
        }
    }
	
    close(sock);

    return 0;
}
