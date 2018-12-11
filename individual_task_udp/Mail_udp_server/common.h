#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <string.h>
#include <string>
#include <vector>
#include <inttypes.h>
#include <thread>
#include <cassert>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <iostream>

using int32 = int32_t;
using uint32 = uint32_t;
using uint64 = uint64_t;
using Socket = int32;
using Package_number = int32;
using Time = uint64;

constexpr int32 Network_port = 5001;
constexpr int32 Message_size_limit = 1024;

constexpr Time Acknowledge_timeout_ms = 5000;

constexpr const char* Acknowledge_prefix = "!ACK";


std::vector<std::string> Split(const std::string& s, char seperator, bool handle_quotes = false, bool remove_quotes = false)
{
	std::vector<std::string> output;
	output.push_back("");

	bool quotes = false;

	int32 pos = 0;
	while (pos < s.size())
	{
		char c = s[pos];

		if (handle_quotes && c == '"')
		{
			quotes = !quotes;
			if (!remove_quotes)
			{
				if (quotes)
				{
					if (output.back() != "") output.push_back("");
					output.back() += "\"";
				}
				else output.back() += "\"";
			}
			else
			{
				if (quotes)
				{
					if (output.back() != "") output.push_back("");
				}
			}
		}
		else
		{
			if (quotes) output.back() += c;
			else
			{
				if (c == seperator)
				{
					if (output.back() != "") output.push_back("");
				}
				else
				{

					output.back() += c;
				}
			}
		}
		++pos;
	}

	if (output.back() == "") output.pop_back();

	return output;
}

struct Message
{
	int32 length{ 0 };
	char message[Message_size_limit + 1] = { 0 };
};

struct Address
{
	std::string hostname;
	int32 port{ 0 };

	std::string to_string() const
	{
		return hostname + ":" + std::to_string(port);
	}

	bool operator==(const Address& other)
	{
		return hostname == other.hostname &&
			port == other.port;
	}

	bool operator!=(const Address& other)
	{
		return !operator==(other);
	}
};

struct Input_message
{
	Address address;
	Message message;
};

struct Package
{
	Package_number number;
	Message message;

	void deserialize(const char buffer[sizeof(number) + sizeof(message)])
	{
		bzero(&message.message, sizeof(message.message));

		int32 off = 0;
		bcopy((char*)buffer, (char*)this, sizeof(number));
		off += sizeof(number);
		bcopy((char*)buffer + off, (char*)this + off, sizeof(message.length));
		off += sizeof(message.length);
		bcopy((char*)buffer + off, (char*)this + off, message.length);

	}

	void serialize(char buffer[sizeof(number) + sizeof(message)], int32& out_size)
	{
		int32 off = 0;
		bcopy((char*)this, (char*)buffer, sizeof(number));
		off += sizeof(number);
		bcopy((char*)this + off, (char*)buffer + off, sizeof(message.length));
		off += sizeof(message.length);
		bcopy((char*)this + off, (char*)buffer + off, message.length);

		out_size = sizeof(number) + sizeof(message.length) + message.length;
	}
};

struct Connection
{
	bool banned{ false };
	Address address;
	int32 number_send{ 0 };
	int32 number_receive{ 0 };

	std::vector<std::pair<Package, Time>> send_sessions;
};

class Server
{
public:
	Server() {}
	Server(const Server&) = delete;
	~Server() {}

	bool start(bool is_server)
	{
		assert(state == State::None);

		this->is_server = is_server;

		address_server.hostname = "127.0.0.1";
		address_server.port = Network_port;

		socket_server = socket(AF_INET, SOCK_DGRAM, 0);
		if (socket_server < 0)
		{
			printf("Failed to open server socket");
			return false;
		}

		sockaddr_in addr_server = { 0 };
		addr_server.sin_family = AF_INET;
		addr_server.sin_addr.s_addr = INADDR_ANY;
		addr_server.sin_port = htons(address_server.port);

		int32 level = 1;
		auto setsockopt_result = setsockopt(socket_server, SOL_SOCKET, SO_BROADCAST, &level, sizeof(level));
		if (setsockopt_result < 0)
		{
			printf("Failed to setsockopt server socket");
			close(socket_server);
			return false;
		}

		if (is_server)
		{
			auto result_bind = bind(socket_server, (sockaddr*)&addr_server, sizeof(addr_server));
			if (result_bind < 0)
			{
				printf("Failed to bind server socket");
				close(socket_server);
				return false;
			}
		}

		state = State::Started;

		return true;
	}

	bool running() const { return state == State::Started; }

	void terminate()
	{
		if (state == State::Started)
		{
			state = State::Terminated;
			shutdown(socket_server, 2);
			close(socket_server);
			socket_server = -1;
			printf("Server terminated\n");
		}
	}

	std::string get_clients()
	{
		std::string result;
		{
			std::lock_guard<std::mutex> _(shared.mutex);

			for (int32 i = 0; i < shared.connections.size(); ++i)
			{
				auto connection = shared.connections[i];
				result += "#" + std::to_string(i) + " at " + connection.address.to_string();
				if (connection.banned) result += " (banned)";
				result += "\n";
			}
		}
		return result;
	}

	bool ban_client(int32 slot)
	{
		{
			std::lock_guard<std::mutex> _(shared.mutex);

			if (slot < 0 || slot >= shared.connections.size()) return false;
			shared.connections[slot].banned = true;
			return true;
		}

		return false;
	}

	void resend_thread()
	{
		assert(state == State::Started);

		while (!terminated)
		{
			{
				std::lock_guard<std::mutex> _(shared.mutex);
				for (auto& connection : shared.connections)
				{
					for (auto& session : connection.send_sessions)
					{
						if (time_ms() - session.second >= Acknowledge_timeout_ms)
						{
							printf(("Package " + std::to_string(session.first.number) + " to " + connection.address.to_string() + " was not acknowledged within timeout, resending\n").c_str());

							send_immediate(connection.address, session.first);
							session.second = time_ms();
						}
					}
				}
			}

			wait_ms(Time{ 100 });
		}
	}

	void listen_thread()
	{
		while (!terminated)
		{
			char buffer[sizeof(Package)];

			sockaddr_in addr = { 0 };
			uint32 addr_size = sizeof(addr);

			int32 n = recvfrom(socket_server, buffer, sizeof(buffer), 0, (sockaddr*)&addr, &addr_size);
			if (n <= 0) break;

			Package package;
			package.deserialize(buffer);

			Address address;
			address.hostname = inet_ntop(AF_INET, &addr.sin_addr, buffer, INET_ADDRSTRLEN);
			address.port = ntohs(addr.sin_port);


			{
				std::lock_guard<std::mutex> _(shared.mutex);

				Connection& connection = obtain_connection(address);
				bool skip = false;
				if (debug_drop_next_input_package)
				{
					skip = true;
					debug_drop_next_input_package = false;
				}
				if (connection.banned || skip)
				{
					printf("Dropping package from %s\n", address.to_string().c_str());
				}
				else
				{
					printf("Processing package from %s\n", address.to_string().c_str());

					bool is_message_acknowledge =
						package.message.length == strlen(Acknowledge_prefix) &&
						bcmp(Acknowledge_prefix, package.message.message, strlen(Acknowledge_prefix)) == 0;

					if (is_message_acknowledge)
					{
						auto& sessions = connection.send_sessions;
						auto it = std::find_if(sessions.begin(), sessions.end(), [&](std::pair<Package, Time> it) {return it.first.number == package.number; });
						if (it == sessions.end())
						{
							printf("No package to acknowledge with number #%s\n", std::to_string(package.number).c_str());
						}
						else
						{
							sessions.erase(it);
							printf("Acknowledged package with number #%s\n", std::to_string(package.number).c_str());
						}
					}
					else
					{
						bool ack = false;
						bool push = false;

						if (package.number > connection.number_receive)
						{
							printf("Dropping package #%s, next package number is #%s\n", std::to_string(package.number).c_str(),
								std::to_string(connection.number_receive).c_str());
						}
						else if (package.number < connection.number_receive)
						{
							printf("Package #%s already received, resending acknowledge\n", std::to_string(package.number).c_str());
							ack = true;
						}
						else
						{
							ack = true;
							push = true;
						}

						if (ack)
						{
							Package package_ack;
							package_ack.number = connection.number_receive;
							bcopy(Acknowledge_prefix, package_ack.message.message, strlen(Acknowledge_prefix));
							package_ack.message.length = strlen(package_ack.message.message);
							send_immediate(address, package_ack);
						}

						if (push)
						{
							Input_message message;
							message.address = address;
							message.message = package.message;
							shared.message_queue.push_back(message);

							++connection.number_receive;
						}
					}

				}
			}

		}
	}

	Connection& obtain_connection(Address address)
	{
		Connection* found_connection = nullptr;

		for (auto& connection : shared.connections)
		{
			if (connection.address != address) continue;

			found_connection = &connection;
			break;
		}
		if (!found_connection)
		{
			Connection connection;
			connection.address = address;
			shared.connections.push_back(connection);
			found_connection = &shared.connections.back();
		}

		return *found_connection;
	}

	// optional address filter
	void send(Address address, std::string in_message)
	{
		{
			std::lock_guard<std::mutex> _(shared.mutex);


			if (in_message.size() == 0) return;

			Connection& connection = obtain_connection(address);

			Package package;
			package.number = connection.number_send;
			bcopy(in_message.data(), package.message.message, in_message.size());
			package.message.length = in_message.size();

			if (debug_disable_next_immediate_send)
			{
				debug_disable_next_immediate_send = false;
			}
			else
			{
				send_immediate(address, package);
			}
			connection.send_sessions.push_back({ package, time_ms() });

			++connection.number_send;
		}

	}

	bool has_message()
	{
		{
			std::lock_guard<std::mutex> _(shared.mutex);

			return shared.message_queue.size() > 0;
		}
	}

	Input_message next_message()
	{
		{
			std::lock_guard<std::mutex> _(shared.mutex);

			assert(shared.message_queue.size() > 0);

			auto message = shared.message_queue.front();
			shared.message_queue.erase(shared.message_queue.begin());
			return message;
		}
	}

	Time time_ms()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}

	void wait_ms(Time length_ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(length_ms));
	}

	bool debug_drop_next_input_package = false; // failure to receive (did not reach the server
	bool debug_disable_next_immediate_send = false; // allows test wrong order & resend

private:


	bool send_immediate(Address address, Package package)
	{
		sockaddr_in target;
		target.sin_family = AF_INET;
		inet_pton(AF_INET, address.hostname.c_str(), &target.sin_addr);
		target.sin_port = htons(address.port);

		char buffer[sizeof(Package)];
		int32 sz;
		package.serialize(buffer, sz);

		int32 send_result = sendto(socket_server, buffer, sz, 0, (const sockaddr*)(&target), sizeof(target));
		if (send_result <= 0)
		{
			printf("Failed to send a package to %s\n", address.to_string().c_str());
			return false;
		}
		return true;
	}


	Address address_server;
	Socket socket_server{ 0 };

	bool is_server{ false };

	bool terminated{ false };

	struct Shared
	{
		std::vector<Connection> connections;
		std::vector<Input_message> message_queue;
		std::mutex mutex;
	};
	Shared shared;

	enum class State
	{
		None,
		Started,
		Terminated
	};
	State state{ State::None };
};
