#include "common.h"

#include <iostream>

int main(int argc, char* argv[]) 
{
	// disable buffering
	setbuf(stdout, NULL);

	Server server;
	Address master_address;
	master_address.hostname = "127.0.0.1";
	master_address.port = Network_port;
	server.start(&master_address);
	
	while(server.running())
	{
		char buffer[Message_size_limit + 1];
		
		std::cin.getline(buffer, Message_size_limit);
		if (buffer[0] == 0) break;

		printf("Now sending: %s\n", buffer);

		server.send(&master_address, std::string(buffer));

		if (!server.running()) break;

		printf("Awaiting response from server...\n");

		Message message;
		server.read_server(message);

		printf("Received from server: %s\n", message.message);
	}

	return 0;
}
