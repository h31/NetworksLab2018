#include "Server.h"

vector<Client*> Server::clients;

Server::Server(uint16_t port){
	struct sockaddr_in serv_addr{};

	/* First call to socket() function */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		closeAndExit("ERROR opening socket", 1);
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	//https://serverfault.com/questions/329845/how-to-forcibly-close-a-socket-in-time-wait
	const int opt = 1;
	if(setsockopt(sockfd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), &opt, sizeof(int)) < 0){
		closeAndExit("ERROR on setsockopt", 1);
	}

	/* Now bind the host address using bind() call.*/
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		closeAndExit("ERROR on binding", 1);
	}

	/* Now start listening for the clients, here process will
	   * go in sleep mode and will wait for the incoming connection
	*/
	listen(sockfd, backlog);

	//connect to database
	db = new DB();

	mainBody();
}

Server::~Server(){
    BOOST_LOG_TRIVIAL(debug) << "server begin destruction";
	for ( Client* n : clients ) {
        delete n;
    }
    clients.clear();
	delete db;
	BOOST_LOG_TRIVIAL(debug) << "server end destruction";
}

void waitForClose(Server* server){
    BOOST_LOG_TRIVIAL(info) << "Сервер успешно запушен";
    BOOST_LOG_TRIVIAL(info) << "Для закрытия сервера нажмите клавишу \'q\'";
	while(getchar() != 'q'){}
	delete server;
    BOOST_LOG_TRIVIAL(info) << "Сервер успешно завершил свою работу";
	exit(0);
}

void Server::mainBody(){

	thread t(waitForClose, this);

	while(true){
		Client* client = new Client();

		if (client->acceptConnection(sockfd) < 0) {
			BOOST_LOG_TRIVIAL(error) << "ERROR on accept";
			delete client;
			continue;
		}

		clients.push_back(client);

		thread t(&Client::handler, client);
		t.detach();
	}
}

void Server::closeAndExit(string errorMsg, int errorCode){
    BOOST_LOG_TRIVIAL(error) << errorMsg;
	delete this;
	exit(errorCode);
}

void Server::deleteClient(Client* client){
	for(int i = 0; i < clients.size(); i++){
		if(client == clients[i]){
			clients.erase(clients.begin() + i);
            BOOST_LOG_TRIVIAL(debug) << "Сlient deleted from vector";
			return;
		}
	}
}