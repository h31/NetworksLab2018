#include "Server.h"
#include "ObjectClass.h"
#include <functional>
#include <iostream>
#include <thread>

Server::Server(int port, int maxClientsCount, int readTimeoutInMilliseconds) 
	: port(port), serverSocket(SafeSocket(port)), maxClientsCount(maxClientsCount), readTimeoutInMilliseconds(readTimeoutInMilliseconds) { }

void Server::start() {
	//ObjectClass obj = ObjectClass(posixAccount);
	//obj.setAttribute(cn, "asd");
	//obj.setAttribute(uid, "123");
	//obj.setAttribute(uidNumber, "456");
	//obj.setAttribute(gidNumber, "789");
	//obj.setAttribute(homeDirectory, "\\123123\\asd");
	//if (obj.isReady()) {
	//	store->addRecord("fix.txt", obj.description());
	//}

	std::thread clientConnectionThread([this] { this->acceptNewClients(); });
	clientConnectionThread.detach();

	std::string command;
	while (true) {
		std::cout << "Write command:\r\n";
		std::cin >> command;
		if (command.compare("q") == 0) {
			break;
		} else if (command.compare("lc") == 0) {
			std::cout << "Active clients(total = " << clientsList.count() << "):\r\n";
			int index = 0;
			clientsList.forEach([index](SafeSocket* clientSocket) mutable {
				std::cout << index++ << ": " << clientSocket << "\r\n"; 
			});
		} else if (command.compare("kick") == 0) {
			int targetID;
			std::cin >> targetID;
			int index = 0;
			SafeSocket* targetSocket = nullptr;
			clientsList.forEach([&](SafeSocket* clientSocket) {
				if (targetID == index) {
					targetSocket = clientSocket;
				}
				++index;
			});

			if (targetSocket == nullptr) {
				continue;
			}

			cleanUpClientConnection(targetSocket);
		}
	}

	closeAllClients();
}

void Server::closeAllClients() {
	serverSocket.close();
	closed = true;
	while (clientsList.count() != 0) {
		Sleep(1000);
	}
}

void Server::acceptNewClients() {
	while (!closed) {
		if (clientsList.count() == maxClientsCount) {
			continue;
		}

		try {
			SafeSocket* client = serverSocket.acceptClient(readTimeoutInMilliseconds);
			clientsList.add(client);
			std::thread handleClientConnectionThread([this, client] { this->handleClientConnection(client); });
			handleClientConnectionThread.detach();
		} catch (std::exception const& e) {
			std::cout << e.what() << "\r\n";
			continue;
		}
	}
}

void Server::handleClientConnection(SafeSocket* clientSocket) {
	while (!closed) {
		// Start reading new message
		char* request;
		try {
			request = clientSocket->readData([this]() { return !this->closed; });
		} catch (std::exception const& e) {
			cleanUpClientConnection(clientSocket);
			return;
		}

		if (request == nullptr) {
			continue;
		}

		char* response = nullptr;
		int responseStatus = 0;

		char* tempBuffer = nullptr;

		char* commandPointer;
		char* firstQuery = strstr(request, "?");
		char* requestEnd = strstr(request, ";");
		if (firstQuery != nullptr && requestEnd != nullptr && requestEnd > firstQuery) {
			requestEnd[0] = '\0';
			try {
				if ((commandPointer = strstr(request, "find")) != nullptr && commandPointer == request) {
					response = store->getRecord(firstQuery + 1);
				} else if ((commandPointer = strstr(request, "add")) != nullptr && commandPointer == request) {
					char* secondQuery = strstr(firstQuery, "&");
					if (secondQuery != nullptr) {
						secondQuery[0] = '\0';
						ObjectClass object = ObjectClass::deserialize(secondQuery + 1);
						tempBuffer = object.description();
						store->addRecord(firstQuery + 1, tempBuffer);
					} else {
						responseStatus = 1;
					}
				} else if ((commandPointer = strstr(request, "delete")) != nullptr && commandPointer == request) {
					store->deleteRecord(firstQuery + 1);
				} else {
					responseStatus = 1;
				}
			} catch (const char* error) {
				response = _strdup(error);
				responseStatus = 1;
			}
		} else {
			responseStatus = 1;
		}

		free(tempBuffer);
		free(request);

		if (response == nullptr) {
			if (responseStatus == 0) {
				response = _strdup("OK");
			} else {
				response = _strdup("Bad request");
			}
		}

		int responseWithStatusSize = 1 + 2 + strlen(response) + 1;
		char* responseWithStatus = (char*)malloc(responseWithStatusSize * sizeof(char));
		sprintf_s(responseWithStatus, responseWithStatusSize, "%d\r\n%s", responseStatus, response);

		free(response);

		// Write a response to the client
		try {
			clientSocket->sendData(responseWithStatus);
		} catch (std::exception const& e) {
			free(responseWithStatus);
			cleanUpClientConnection(clientSocket);
			return;
		}

		free(responseWithStatus);
	}

	// Server is finishing -> cleaning up
	cleanUpClientConnection(clientSocket);
}

void Server::cleanUpClientConnection(SafeSocket* clientSocket) {
	clientSocket->close();
	clientsList.remove(clientSocket);
	delete clientSocket;
}

Server::~Server() {
	clientsList.forEach([](SafeSocket* clientSocket) {
		delete clientSocket;
	});
	delete store;
}