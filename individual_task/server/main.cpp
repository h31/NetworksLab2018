#include "common.h"

#include <fstream>

constexpr char* Login_file = "logins.txt"; // файл для хранения логинов

constexpr int32 Header_length = 16; // макс. длинна для предпросмотр. сообщения
constexpr int32 Name_limit = 64; // макс. длинна имени

constexpr const char* Available_commands = "Available commands:\nlist\nkick <slot>\nexit"; // доступные команды на сервере

struct Mail_box // почт. ящик
{
	std::string owner; // владелец
	std::vector<std::string> letters; // список писем
};

struct Mail_user
{
	std::string username;
};

class Mail // функционал почты
{
public:
	Mail()
	{
		std::ifstream in(Login_file); // ввод в файл 
		std::string line;
		while (std::getline(in, line))
		{
			if (line.size() > 0) get_box(line);
		}
	}
	Mail(const Mail&) = delete;
	~Mail()
	{
		std::ofstream out(Login_file); // вывод из файла
		for (auto& box : mail_box_vector)
		{
			out << box.owner << "\n";
		}
	}

	bool read_message(std::string user, int32 offset, std::string& out) // счиатать почту такого юзера, под таким номером
	{
		Mail_box& box = get_box(user); // созд. ящик

		if (offset < 0 || offset >= box.letters.size()) return false; // если номер

		out = box.letters[offset];
		return true;
	}

	std::string list_messages(std::string user) // список писем пользователя
	{
		std::string result = "Current messages for " + user + ":\n";

		Mail_box& box = get_box(user); // созд. ящик

		for (int32 i = 0, size = box.letters.size(); i != size; ++i) // пред. просмотр
		{
			std::string message = box.letters[i];
			int32 header_length = std::min((int32)message.size(), Header_length); // 16 симв. или меньше
			std::string header = message.substr(0, header_length);
			std::replace(header.begin(), header.end(), '\n', ' '); // заменяем перенос пробелом
			if (i != 0) result += "\n";
			result += std::to_string(i) + ": " + header;
		}

		return result;
	}

	bool send_message(std::string user, std::string message, std::string targets) // отправить писмо от этого юзера, с таким сообщением
	{
		std::string letter = construct_letter(user, message);

		auto target_vector = Split(targets, ';', true, true); // если неск. - получатели через ;
		if (target_vector.size() == 0) return false;

		for (auto& target : target_vector)
		{
			if (!has_box(target)) continue;
			Mail_box& box = get_box(target);
			box.letters.push_back(letter);
		}
		return true;
	}

	bool delete_message(std::string user, int32 offset) // юзера, под таким номером
	{
		Mail_box& box = get_box(user);

		if (offset < 0 || offset >= box.letters.size()) return false;

		box.letters.erase(box.letters.begin() + offset); // стерает
		return true;
	}

	bool get_user(Address address, std::string& out) // hostname, port
	{
		for (auto& pair : connections) // какому юзеру принадлежит ip адрес
		{
			if (pair.first == address)
			{
				out = pair.second.username;
				return true;
			}
		}
		return false;
	}

	bool login(Address address, std::string name) // привяз. ip адрес с портом к какому то имени
	{
		if (get_user(address, name)) return false; // socket already connected

		Mail_user user;
		user.username = name; // создать юзера
		
		get_box(name);
		connections.push_back(std::make_pair(address, user)); // добавл. в список соединений
		return true;
	}

private:
	std::string construct_letter(std::string user, std::string message)
	{
		return message + "\n\nReceived from " + user;
	}
	
	bool has_box(std::string owner)
	{
		for (auto& box : mail_box_vector)
		{
			if (box.owner == owner) return true;
		}
		return false;
	}

	Mail_box& get_box(std::string owner)
	{
		for (auto& box : mail_box_vector)
		{
			if (box.owner == owner) return box;
		}
		Mail_box box;
		box.owner = owner;
		mail_box_vector.push_back(box);
		return mail_box_vector.back();
	}

	std::vector<Mail_box> mail_box_vector;
	std::vector<std::pair<Address, Mail_user>> connections;
};

void master(Server& server) // команды на сервере
{
	while (server.running())
	{
		std::string command;
		std::getline(std::cin, command);

		if (command == "help") // выводит все команды
		{
			printf("%s\n", Available_commands);
		}
		else if (command == "list") // список соединений
		{
			std::string list = server.get_clients();
			printf("Current connections:\n%s", list.c_str());
		}
		else if (command.find("kick") != std::string::npos) //откл. соед.
		{
			int32 number = std::stoi(command.substr(5, command.size() - 5));
			bool kicked = server.kick_client(number);
			if (!kicked)
			{
				printf("Could not terminate connection\n");
			}
		}
		else if (command == "exit") //выход
		{
			server.terminate();
		}
		else
		{
			printf("Unknown command\n");
		}

	}
}

class Mail_processor : public Message_processor
{
public:
	Mail_processor(Server& server, Mail& mail) :
		server(server), mail(mail) {}
	Mail_processor(const Mail_processor&) = delete;
	~Mail_processor() override {}

	void process(std::string message, Address from) override
	{
		auto tokens = Split(message, ' ', true, true); // работа с пробелом

		std::string name;
		bool logged = mail.get_user(from, name);
		if (tokens.size() == 2) // токены
		{
			auto first = tokens[0];
			auto second = tokens[1];

			if (first == "LOGIN")
			{
				if (second.size() == 0 || second.size() > Name_limit)
				{
					server.send(&from, "Bad name: " + second);
					return;
				}


				if (logged)
				{
					server.send(&from, "Already logged in as: " + name); // повторный логин
					return;
				}

				logged = mail.login(from, second);
				if (!logged)
				{
					server.send(&from, "Failed to log in as: " + second);
					return;
				}

				server.send(&from, "Logged in as: " + second);
				return;
			}
		}

		if (!logged)
		{
			server.send(&from, "Please log in"); // если первоначально не залогинелся
			return;
		}

		if (tokens.size() == 1)
		{
			auto first = tokens[0];
			if (first == "LIST")
			{
				server.send(&from, mail.list_messages(name));
				return;
			}
		}

		if (tokens.size() == 2)
		{
			auto first = tokens[0];
			auto second = tokens[1];

			if (first == "READ")
			{
				std::string message;
				if (!mail.read_message(name, std::stoi(second), message))
				{
					server.send(&from, "Letter not found"); // чтение несущ. сообщения
					return;
				}

				server.send(&from, message);
				return;
			}
			if (first == "DELETE")
			{
				if (!mail.delete_message(name, std::stoi(second)))
				{
					server.send(&from, "Letter not found"); // удаление несущ. сообщения
					return;
				}

				server.send(&from, "Deleted successfully");
				return;
			}
		}

		if (tokens.size() == 3)
		{
			auto first = tokens[0];
			auto second = tokens[1];
			auto third = tokens[2];

			if (first == "SEND")
			{
				if (!mail.send_message(name, second, third))
				{
					server.send(&from, "Failed to send message");
					return;
				}
				server.send(&from, "Message sent");
				return;
			}
		}
		server.send(&from, "Unexpected package");
		printf("Unexpected package received from %s: %s\n", from.to_string().c_str(), message.c_str());
	}

private:
	Server & server;
	Mail& mail;
};

int main(int argc, char* argv[])
{
	// disable buffering
	setbuf(stdout, NULL);


	Mail mail; // созд. почту
	Server server; //созд. сервер
	Mail_processor processor{ server, mail };
	server.start(nullptr, &processor); // мы сервер и к нам коннектятся


	std::thread accept_thread([&] {server.accept_thread(); }); // обработчик сообщений пришедших на сервер
	std::thread master_thread([&] {master(server); });
	printf("%s\n", Available_commands);

	accept_thread.join();
	master_thread.join();

	printf("Press any key to exit...");
	std::cin.get();

	return 0;
}
