#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <conio.h>

#define MAX_THREADS 10
#define USING_THREADS 2
#define PORT_SERVER 8080
#define SIZE_PACKET 8

#pragma comment(lib,"Ws2_32.lib")

using namespace std;

//Funtions of the Server
int main(int argc, char *argv[]);
int MessageReceive(SOCKET socket, char *buf, string &line);
int MessageSend(SOCKET socket, string buffer);
DWORD WINAPI ProcessClient(void* socket);
DWORD WINAPI ProcessServer();
DWORD WINAPI ConnectionsAccept(void *listenSocket);
void WSAStart();
void closeSocket(int ind);

//Functions of the terminal
int UserFileRewrite();
int getIndexUser(string login);
int UserFileRead();
int addusrImpl(string login, string password);
int loginImpl(string login, string password);
int killImpl(string login);
string getServerPath();
string cdImpl(SOCKET sock, string dir, string path);
vector<string> lsImpl(string folder);
bool DirCompare(string i, string j);