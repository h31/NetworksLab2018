#include "common.h"

constexpr const char* Available_commands = "Available commands:\nlogin <name>\nsend <message> <recepients>\nlist\nread <number>\ndelete <number>";
// возможные команды на клиенте
bool construct_command(std::string input, std::string& out) // польз. команды (сообщ. протокола мал. буквами )
{
	auto tokens = Split(input, ' ', true, false); // обработка пробела

	if (tokens.size() == 1)
	{
		auto first = tokens[0];

		if (first == "list")
		{
			out = "LIST";
			return true;
		}
	}

	if (tokens.size() == 2)
	{
		auto first = tokens[0];
		auto second = tokens[1];

		if (first == "login")
		{
			out = "LOGIN " + second;
			return true;
		}

		if (first == "read")
		{
			out = "READ " + second;
			return true;
		}

		if (first == "delete")
		{
			out = "DELETE " + second;
			return true;
		}
	}

	if (tokens.size() == 3)
	{
		auto first = tokens[0];
		auto second = tokens[1];
		auto third = tokens[2];

		if (first == "send")
		{
			out = "SEND " + second + " " + third;
			return true;
		}
	}

	return false;
}

int main(int argc, char* argv[]) 
{
	// disable buffering
	setbuf(stdout, NULL);

	Server server;

	Address master_address;
	master_address.hostname = "127.0.0.1"; // к чему коннектимся
	master_address.port = Network_port;
	server.start(&master_address, nullptr); // стартует в режиме клиента (коннектится)

	printf("%s\n", Available_commands);
	
	while(server.running()) // пока сервер работает
	{
		char buffer[Message_size_limit + 1];
		
		std::cin.getline(buffer, Message_size_limit);
		if (buffer[0] == 0) break;

		std::string command;
		if (!construct_command(buffer, command)) // если введена неизвестная команда
		{
			printf("Incorrect command. %s\n", Available_commands);
			continue;
		}

		//printf("Now sending: %s\n", buffer);

		server.send(&master_address, command);

		if (!server.running()) break; // если сервер не работает

		//printf("Awaiting response from server...\n");

		Message message;
		server.read_server(message);

		printf("%s\n", message.message);
	}

	return 0;
}
