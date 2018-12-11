#include "common.h"

constexpr const char* Available_commands = "Available commands:\nlogin <name>\nsend <message> <recepients>\nlist\nread <number>\ndelete <number>\nexit\n";

bool construct_command(std::string input, std::string& out)
{
	auto tokens = Split(input, ' ', true, false);

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

void master(Server& server)
{
	while (server.running())
	{
		std::string command;
		std::getline(std::cin, command);

		if (command == "help")
		{
			printf(Available_commands);
		}
		else if (command == "exit")
		{
			server.terminate();
		}
		else
		{
			std::string protocol_command;
			if (!construct_command(command, protocol_command))
			{
				printf("Incorrect command. %s\n", Available_commands);
				continue;
			}

			Address address;
			address.hostname = "127.0.0.1";
			address.port = Network_port;
			server.send(address, protocol_command);
		}

	}
}

void logic(Server& server)
{
	while (server.running())
	{
		if (server.has_message())
		{
			auto message = server.next_message();
			// std::string str = "Received " + std::string(message.message.message) + " from " + message.address.to_string() + "\n";
			std::cout << message.message.message << "\n";
		}

		server.wait_ms(Time{ 50 });
	}
}

int main(int argc, char* argv[])
{
	Server server;
	server.start(false);

	std::thread listen_thread([&] {server.listen_thread(); });
	std::thread resend_thread([&] {server.resend_thread(); });
	std::thread logic_thread([&] {logic(server); });
	std::thread master_thread([&] {master(server); });
	printf(Available_commands);

	// server.debug_disable_next_immediate_send = true;

	listen_thread.join();
	resend_thread.join();
	logic_thread.join();
	master_thread.join();

	printf("Press any key to exit...");
	std::cin.get();

	return 0;
}
