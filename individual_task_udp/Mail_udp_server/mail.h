#pragma once

#include "common.h"

#include <fstream>

constexpr char* Login_file = "logins.txt";

constexpr int32 Header_length = 16;
constexpr int32 Name_limit = 64;


struct Mail_box
{
	std::string owner;
	std::vector<std::string> letters;
};

struct Mail_user
{
	std::string username;
};

class Mail
{
public:
	Mail() 
	{
		std::ifstream in(Login_file);
		std::string line;
		while (std::getline(in, line))
		{
			if (line.size() > 0) get_box(line);
		}
	}
	Mail(const Mail&) = delete;
	~Mail() 
	{
		std::ofstream out(Login_file);
		for (auto& box : mail_box_vector)
		{
			out << box.owner << "\n";
		}
	}

	bool read_message(std::string user, int32 offset, std::string& out)
	{
		Mail_box& box = get_box(user);

		if (offset < 0 || offset >= box.letters.size()) return false;

		out = box.letters[offset];
		return true;
	}

	std::string list_messages(std::string user)
	{
		std::string result = "Current messages for " + user + ":\n";

		Mail_box& box = get_box(user);

		for (int32 i = 0, size = box.letters.size(); i != size; ++i)
		{
			std::string message = box.letters[i];
			int32 header_length = std::min((int32)message.size(), Header_length);
			std::string header = message.substr(0, header_length);
			std::replace(header.begin(), header.end(), '\n', ' ');
			if (i != 0) result += "\n";
			result += std::to_string(i) + ": " + header;
		}

		return result;
	}

	bool send_message(std::string user, std::string message, std::string targets)
	{
		std::string letter = construct_letter(user, message);

		auto target_vector = Split(targets, ';', true, true);
		if (target_vector.size() == 0) return false;

		for (auto& target : target_vector)
		{
			if (!has_box(target)) continue;
			Mail_box& box = get_box(target);
			box.letters.push_back(letter);
		}
		return true;
	}

	bool delete_message(std::string user, int32 offset)
	{
		Mail_box& box = get_box(user);

		if (offset < 0 || offset >= box.letters.size()) return false;

		box.letters.erase(box.letters.begin() + offset);
		return true;
	}

	bool get_user(Address address, std::string& out)
	{
		for (auto& pair : connections)
		{
			if (pair.first == address)
			{
				out = pair.second.username;
				return true;
			}
		}
		return false;
	}

	bool login(Address address, std::string name)
	{
		if (get_user(address, name)) return false; // socket already connected

		Mail_user user;
		user.username = name;

		get_box(name);
		connections.push_back(std::make_pair(address, user));
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


class Mail_processor
{
public:
	Mail_processor(Server& server, Mail& mail) :
		server(server), mail(mail) {}
	Mail_processor(const Mail_processor&) = delete;
	~Mail_processor() {}

	void process(std::string message, Address from)
	{
		auto tokens = Split(message, ' ', true, true);

		std::string name;
		bool logged = mail.get_user(from, name);
		if (tokens.size() == 2)
		{
			auto first = tokens[0];
			auto second = tokens[1];

			if (first == "LOGIN")
			{
				if (second.size() == 0 || second.size() > Name_limit)
				{
					server.send(from, "Bad name: " + second);
					return;
				}


				if (logged)
				{
					server.send(from, "Already logged in as: " + name);
					return;
				}

				logged = mail.login(from, second);
				if (!logged)
				{
					server.send(from, "Failed to log in as: " + second);
					return;
				}

				server.send(from, "Logged in as: " + second);
				return;
			}
		}

		if (!logged)
		{
			server.send(from, "Please log in");
			return;
		}

		if (tokens.size() == 1)
		{
			auto first = tokens[0];
			if (first == "LIST")
			{
				server.send(from, mail.list_messages(name));
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
					server.send(from, "Letter not found");
					return;
				}

				server.send(from, message);
				return;
			}
			if (first == "DELETE")
			{
				if (!mail.delete_message(name, std::stoi(second)))
				{
					server.send(from, "Letter not found");
					return;
				}

				server.send(from, "Deleted successfully");
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
					server.send(from, "Failed to send message");
					return;
				}
				server.send(from, "Message sent");
				return;
			}
		}
		server.send(from, "Unexpected package");
		printf("Unexpected package received from %s: %s\n", from.to_string().c_str(), message.c_str());
	}

private:
	Server & server;
	Mail& mail;
};
