#include "server.h"




struct DescriptClient{
    SOCKET sock;
    HANDLE handle;
    char *ip;
    int port;
    string login;
};

struct UserAuth{
    string login;
    string password;
    string permissions;
    string path;
    bool online;
};

int ThreadsMax;
int PortServer;
int SizePacket;

DescriptClient clientDesc[MAX_THREADS];
SOCKET listenSocket = INVALID_SOCKET;
const HANDLE hMutex = CreateMutex(NULL, false, NULL);

enum state{ CONNECT, WORK };
vector<UserAuth> users;
string allPermissions = "sudo";

// Server Functions
void closeSocket(int ind){
    WaitForSingleObject(hMutex, INFINITE);
    if(ind >= 0 && ind < ThreadsMax && clientDesc[ind].sock != INVALID_SOCKET){
            if(closesocket(clientDesc[ind].sock))
                puts("ERROR Closesocket");
            else{
                cout << clientDesc[ind].ip << ":" << clientDesc[ind].port << " was disconnected" << endl;
                clientDesc[ind].sock = INVALID_SOCKET;
            }
    }
    ReleaseMutex(hMutex);
}

int MessageReceive(SOCKET socket, char *buf, string &line){
    int rc = 1;
    while(rc != 0){
        rc = recv(socket, buf, 1, 0);
        if(rc <= 0) return 0;
        if(buf[0] == '\n') return 1;
        line = line + buf[0];
    }
}

int MessageSend(SOCKET socket, string buffer){
    if(send(socket, buffer.data(), buffer.size(), 0) <= 0) return 0;
    return 1;
}

DWORD WINAPI ProcessClient(void* socket){
    int ind;
    bool exitFlag = false;
    state state = CONNECT;
    char buffer[SizePacket];
    string line;

    for (int i = 0; i < ThreadsMax; i++){
        if (clientDesc[i].sock == (SOCKET)socket) ind = i;
    }

    while(!exitFlag){
        string login;
        string password;
        string cmd;
        int logInd;
        size_t pos = 0;
        line = "";

        switch(state){
            case CONNECT:

                MessageSend(clientDesc[ind].sock, "You can login or register\nlogin 'Your login' 'Your password' - to login\naddusr 'Your login' 'Your password' - to create new user and login\n");

                if(MessageReceive(clientDesc[ind].sock, buffer, line) != 1){
                    exitFlag = true;
                    break;
                }else cout << clientDesc[ind].ip << ":" << clientDesc[ind].port << " " << line << endl;

                pos = line.find(" ");
                cmd = line.substr(0, pos);

                if(cmd == "login" || cmd == "addusr"){
                    if(line.find_first_of(" ") != string::npos)
                        line = line.substr(line.find_first_of(" "), string::npos);
                    if(line.find_first_not_of(" ") != string::npos)
                        line = line.substr(line.find_first_not_of(" "), string::npos);
                    
                    pos = line.find(" ");
                    login = line.substr(0, pos);

                    if(line.find_first_of(" ") != string::npos)
                        line = line.substr(line.find_first_of(" "), string::npos);
                    if(line.find_first_not_of(" ") != string::npos)
                        line = line.substr(line.find_first_not_of(" "), string::npos);

                    pos = line.find(" ");
                    password = line.substr(0, pos);
                }
                else MessageSend(clientDesc[ind].sock, "Invalid command!\n");
                
                if(cmd == "login"){
                    if(loginImpl(login, password) == 0){
                        logInd = getIndexUser(login);

                        WaitForSingleObject(hMutex,INFINITE);
                        clientDesc[ind].login = login;
                        ReleaseMutex(hMutex);

                        state = WORK;
                    }
                    else if(loginImpl(login, password) == 1){
                        MessageSend(clientDesc[ind].sock, "Login/password not match!\n");
                        break;
                    }
                    else{
                        MessageSend(clientDesc[ind].sock, "This user already online!\n");
                        break;                        
                    }
                }

                if(cmd == "addusr"){
                    if(addusrImpl(login, password) == 0){
                        logInd = getIndexUser(login);

                        WaitForSingleObject(hMutex,INFINITE);
                        clientDesc[ind].login = login;
                        users[logInd].online = true;
                        ReleaseMutex(hMutex);

                        state = WORK;
                    }
                    else if(addusrImpl(login, password) == 1){
                        MessageSend(clientDesc[ind].sock, "This user already exists!\n");
                        break;
                    }
                    else if(addusrImpl(login, password) == 2){
                        cout << "ERROR: Can't open listusers.txt" << endl;
                        return 2;
                    }
                    else{
                        MessageSend(clientDesc[ind].sock, "Matching login/password are not allowed!\n");
                    }
                }

                break;
            case WORK:
                while(true){
                    string cmd;
                    vector<string> names;

                    string prompt = users[logInd].login + " @ " + users[logInd].path + " $ \n";
                    MessageSend(clientDesc[ind].sock, prompt);
                    
                    cmd = "";
                    if(MessageReceive(clientDesc[ind].sock, buffer, cmd) != 1){
                        exitFlag = true;
                        break;
                    }else cout << clientDesc[ind].ip << ":" << clientDesc[ind].port
                               << "|" << clientDesc[ind].login << "| " << cmd << endl;

                    if(cmd == "ls"){
                        names = lsImpl(users[logInd].path);

                        for(int i = 0; i < names.size(); i++)
                            MessageSend(clientDesc[ind].sock, names[i] + "\n");
                        names.clear();
                    }

                    else if(cmd.find("cd ") != string::npos){
                        cmd = cmd.substr(3, string::npos);
                        cmd = cmd.substr(0, cmd.find_last_not_of(" ") + 1);

                        users[logInd].path = cdImpl(clientDesc[ind].sock, cmd, users[logInd].path);
                        UserFileRewrite();
                    }


                    else if(cmd == "who"){
                        if(users[logInd].permissions.find("sudo") != string::npos){
                            for(int i = 0; i < users.size(); i++){
                                string online, send;
                                if(users[i].online) online = "ONLINE";
                                send = users[i].login + "\t" + online + "\t" + users[i].path + "\n";
                                MessageSend(clientDesc[ind].sock, send);
                            }
                        }else MessageSend(clientDesc[ind].sock, "You doesn't have permissions for this command!\n");
                    }


                    else if(cmd.find("kill ") != string::npos){
                        if(users[logInd].permissions.find("sudo") != string::npos){
                            cmd = cmd.substr(5, string::npos);
                            cmd = cmd.substr(0, cmd.find_last_not_of(" ") + 1);

                            if(cmd == "root" || cmd == users[logInd].login){
                                MessageSend(clientDesc[ind].sock, "You can't kill root and yourself!\n");
                                continue;
                            }

                            if(killImpl(cmd) != 0) MessageSend(clientDesc[ind].sock, "This user is offline or not exists!\n");
                        }else MessageSend(clientDesc[ind].sock, "You doesn't have permissions for this command!\n");
                    }

                    else if(cmd == "logout"){
                        break;
                    }

                    else MessageSend(clientDesc[ind].sock, "Invalid command!\n");
                }

                WaitForSingleObject(hMutex,INFINITE);
                users[logInd].online = false;
                clientDesc[ind].login = "";
                ReleaseMutex(hMutex);

                state = CONNECT;

                break;
				default:
                break;
        }
    }

    closeSocket(ind);
    return 0;
}

DWORD WINAPI ProcessServer(){
    while(true){
        string inputServer;
        cin >> inputServer;

        if(inputServer == "q"){
            puts("Stopping server...");
            shutdown(listenSocket, SD_BOTH);
            closesocket(listenSocket);

            return 0;
        }else if(inputServer == "l"){
            puts("List of clients: ");

            WaitForSingleObject(hMutex, INFINITE);
            int lineCount = 0;
            for(int i = 0; i < ThreadsMax; i++){
                if(clientDesc[i].sock != INVALID_SOCKET){
                    cout << i << "|" << clientDesc[i].ip << ":" << clientDesc[i].port << "|" << clientDesc[i].login << endl;
                    lineCount++;
                }
            }
            if(lineCount == 0) puts("empty");
            ReleaseMutex(hMutex);
        }else if(inputServer == "k"){
            SOCKET sock;
            cin >> sock;
            if(cin.fail()) cin.clear();
            else closeSocket(sock);
        }
    }

    return 0;
}

DWORD WINAPI ConnectionsAccept(void *listenSocket){
    SOCKET acceptSocket;
    DescriptClient desc;
    sockaddr_in clientInfo;
    int clientInfoSize = sizeof(clientInfo);

    while(true){
        int ind = -1;

        acceptSocket = accept((SOCKET)listenSocket, (struct sockaddr*)&clientInfo, &clientInfoSize);

        if(acceptSocket == INVALID_SOCKET) break;
        else if(acceptSocket < 0){
            puts("ERROR Accept");
            continue;
        }

        WaitForSingleObject(hMutex, INFINITE);
        for (int i = ThreadsMax - 1; i >= 0; i--)
        if(clientDesc[i].sock == INVALID_SOCKET){
            ind = i;
        }
        ReleaseMutex(hMutex);

        if(ind < 0){
            char *ip  = inet_ntoa(clientInfo.sin_addr);

            MessageSend(acceptSocket, "\nERROR, Server is full, try to connect later o/\n");
            shutdown(acceptSocket, SD_BOTH);
            closesocket(acceptSocket);

            cout << ip << ":" << clientInfo.sin_port << " was disconnected because of server overload" << endl;
            continue;
        }

        desc.sock = acceptSocket;
        desc.ip   = inet_ntoa(clientInfo.sin_addr);
        desc.port = clientInfo.sin_port;

        cout << "Connection request received." << endl;
        cout << "New socket was created at address " << desc.ip << ":" << desc.port << endl;

        clientDesc[ind] = desc;
        clientDesc[ind].handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ProcessClient, (void *)desc.sock, 0, NULL);
    }

    WaitForSingleObject(hMutex, INFINITE);
    vector<HANDLE> hClients;
    for(int i = 0; i < ThreadsMax; i++)
        if(clientDesc[i].sock != INVALID_SOCKET){
            hClients.push_back(clientDesc[i].handle);
            closeSocket(i);
        }
    ReleaseMutex(hMutex);

    if (!hClients.empty()){
        WaitForMultipleObjects(hClients.size(), hClients.data(), TRUE, INFINITE);
    }
    return 0;
}

void WSAStart(){
    WSADATA wsaDATA;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaDATA);
    if(iResult != 0){
        printf("ERROR WSAStartup: %d\n", iResult);
        exit(1);
    }
}

int main(int argc, char *argv[]){
  

    if(ThreadsMax == 0) ThreadsMax = USING_THREADS;
    if(PortServer == 0) PortServer = PORT_SERVER;
    if(SizePacket == 0) SizePacket = SIZE_PACKET;

    cout << "Setting of the Server"         << endl;
    cout << "Number of Threads: " << ThreadsMax << endl;
    cout << "Port Number:    " << PortServer << endl;
    cout << "Size of Buffer:  " << SizePacket << endl;
    cout << endl;

    if(UserFileRead() == 1){
        cout << "ERROR: Can't open listlistusers.txt, creating new file with root:admin user" << endl;

        WaitForSingleObject(hMutex, INFINITE);
        ofstream ofs;
        ofs.open("listusers.txt");
        ofs << "root||admin||" << allPermissions << "||" << getServerPath() << endl;
        ofs.close();
        ReleaseMutex(hMutex);

        if(UserFileRead() == 1){
            cout << "ERROR: Can't open listusers.txt" << endl;
        }
    }

    WSAStart();

    WaitForSingleObject(hMutex, INFINITE);
    for (int i = 0; i < ThreadsMax; i++)
        clientDesc[i].sock = INVALID_SOCKET;
    ReleaseMutex(hMutex);

    sockaddr_in server;
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port        = htons(PortServer);
    server.sin_family      = AF_INET;

    if (listenSocket < 0){
        puts("ERROR Socket ");
        WSACleanup();
        exit(2);
    }

    if(bind(listenSocket, (struct sockaddr *) &server, sizeof(server)) < 0){
        puts("ERROR Bind");
        exit(3);
    }

    if(listen(listenSocket, SOCKET_ERROR) == SOCKET_ERROR){
        puts("ERROR Listen");
        exit(4);
    }

    HANDLE hAccept = CreateThread(NULL, 0, ConnectionsAccept, (void *)listenSocket, 0, NULL);

    ProcessServer();

    WaitForSingleObject(hAccept, INFINITE);

    WSACleanup();
    return 0;
}

//Functions of terminal

bool DirCompare(string i, string j){
    bool iDir = false;
    bool jDir = false;

    if(i.find("/") != string::npos) iDir = true;
    if(j.find("/") != string::npos) jDir = true;

    if(iDir && !jDir) return 1;
    if(!iDir && jDir) return 0;
    else{
        if(i<j) return 1;
        else return 0;
    }
}

string getServerPath(){
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    string::size_type pos = string(buffer).find_last_of("\\/");
    return string(buffer).substr(0, pos);
}

int UserFileRewrite(){
    WaitForSingleObject(hMutex, INFINITE);
    ofstream ofs;
    string filePath = getServerPath() + "/listusers.txt";

    ofs.open(filePath.data());

    if(ofs.is_open()){
        for(int i = 0; i < users.size(); i++){
            ofs << users[i].login << "||" << users[i].password << "||"
                << users[i].permissions << "||" << users[i].path << endl;
        }
    }else{
        ReleaseMutex(hMutex);

        return 1;
    }

    ofs.close();
    ReleaseMutex(hMutex);

    return 0;
}

vector<string> lsImpl(string folder){
    vector<string> names;
    string search_path = folder + "/*.*";
    WIN32_FIND_DATA fd; 
    HANDLE hFind = FindFirstFile(search_path.c_str(), &fd); 
    if(hFind != INVALID_HANDLE_VALUE){ 
        do{
            if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){
                names.push_back(fd.cFileName);
            }
            else{
                string dir = fd.cFileName;
                if(!(dir == "." || dir == "..")){
                    dir += "/";
                    names.push_back(dir);
                }
            }
        }while(FindNextFile(hFind, &fd)); 
        FindClose(hFind);

        sort(names.begin(), names.end(), DirCompare);
    } 
    return names;
}

string cdImpl(SOCKET sock, string dir, string path){
    vector<string> names;
    string subdir;

    size_t pos = dir.find_last_of("/");
    if(pos != dir.size() - 1) dir += "/";
    pos = 0;
    
    while((pos = dir.find("/")) != string::npos){
        subdir = dir.substr(0, pos);
        if(subdir == "..") path = path.substr(0, path.find_last_of("\\/"));
        else{
            names = lsImpl(path);
            if(find(names.begin(), names.end(), subdir + "/") != names.end())
                path += "/" + subdir;
            else{
                string msg = "ERROR Directory \"" + subdir + "\" is not exist\n";
                MessageSend(sock, msg);
                names.clear();
                break;
            }
            names.clear();
        }
        dir.erase(0, pos + 1);
    }

    return path;
}

int getIndexUser(string login){
    int ind = -1;

    WaitForSingleObject(hMutex, INFINITE);
    for(int i = 0; i < users.size(); i++){
        if(users[i].login == login){
            ind = i;
            break;
        }
    }
    ReleaseMutex(hMutex);

    return ind;
}

int UserFileRead(){
    ifstream ifs;
    string line;
    string filePath = getServerPath() + "/listusers.txt";

    WaitForSingleObject(hMutex, INFINITE);
    ifs.open(filePath.data());

    if(ifs.is_open()){
        while(getline(ifs, line)){
            size_t pos = 0;
            string subline;
            vector<string> tempStrings;
            UserAuth ud;

            while((pos = line.find("||")) != string::npos){
                subline = line.substr(0, pos);
                tempStrings.push_back(subline);
                line.erase(0, pos + 2);
            }
            tempStrings.push_back(line);

            ud.login       = tempStrings[0];
            ud.password    = tempStrings[1];
            ud.permissions = tempStrings[2];
            ud.path        = tempStrings[3];
            ud.online      = false;

            users.insert(users.end(), ud);
        }
    }else{
        ReleaseMutex(hMutex);

        return 1;
    }

    ifs.close();
    ReleaseMutex(hMutex);

    return 0;
}

int addusrImpl(string login, string password){
    WaitForSingleObject(hMutex, INFINITE);
    ofstream ofs;
    int ind = getIndexUser(login);
    string filePath = getServerPath() + "/listusers.txt";

    if(ind >= 0){
        return 1;
    }

    if(login == password) return 3;

    ofs.open(filePath.data(), ofstream::app);

    if(ofs.is_open()){
        ofs << login << "||" << password << "||||" << getServerPath() << endl;
    }else{
        ReleaseMutex(hMutex);

        return 2;
    }

    ofs.close();
    ReleaseMutex(hMutex);

    UserAuth ud;

    ud.login       = login;
    ud.password    = password;
    ud.permissions = "";
    ud.path        = getServerPath();
    ud.online      = false;

    WaitForSingleObject(hMutex, INFINITE);
    users.insert(users.end(), ud);
    ReleaseMutex(hMutex);
    
    return 0;
}


int killImpl(string login){
    int ind = -1;

    WaitForSingleObject(hMutex,INFINITE);
    for(int i = 0; i < ThreadsMax; i++){
        if(clientDesc[i].login == login){
            ind = i;
            break;
        }
    }

    if(ind >= 0){
        users[getIndexUser(login)].online = false;
        clientDesc[ind].login = "";
        ReleaseMutex(hMutex);

        closeSocket(ind);
    }else{
        ReleaseMutex(hMutex);
        return 1;
    }

    return 0;
}

int loginImpl(string login, string password){
    int ind = getIndexUser(login);

    if(ind >= 0){
        if(users[ind].online == true) return 2;
        if(users[ind].password == password){
            WaitForSingleObject(hMutex, INFINITE);
            users[ind].online = true;
            ReleaseMutex(hMutex);
        }else return 1;
    }else return 1;

    return 0;
}