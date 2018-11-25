#include "common.h"

constexpr const char* Available_commands = "Available commands:\nlist\nkick <slot>\nexit\n";

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
		else if (command == "list")
		{
			std::string list = server.get_clients();
			printf("Current connections:\n%s", list.c_str());
		}
		else if (command.find("kick") != std::string::npos)
		{
			int32 number = std::stoi(command.substr(5, command.size() - 5));
			bool kicked = server.kick_client(number);
			if (!kicked)
			{
				printf("Could not terminate connection\n");
			}
		}
		else if (command == "exit")
		{
			server.terminate();
		}
		else
		{
			printf("Unknown command\n");
		}

	}
}

int main(int argc, char* argv[])
{
	Server server;
	server.start(nullptr);

	std::thread accept_thread([&] {server.accept_thread(); });
	std::thread master_thread([&] {master(server); });
	printf(Available_commands);



	accept_thread.join();
	master_thread.join();

	printf("Press any key to exit...");
	std::cin.get();

	return 0;
}
