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
#include <iostream>

using int32 = int32_t;
using uint32 = uint32_t;
using Socket = int32;

constexpr int32 Network_port = 5001;
constexpr int32 Message_size_limit = 1024;

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
};

struct Shared_data
{
	std::vector<std::thread*> threads;
	std::vector<Socket> sockets;
	std::vector<Address> addresses;
	int32 listen_count{ 0 };

	std::thread* accept_thread{ nullptr };

	std::mutex mutex;
};

Shared_data shared;

class Server
{
public:
	Server() {}
	Server(const Server&) = delete;
	~Server() {}

	bool start(Address* master_address)
	{
		assert(state == State::None);

		bool is_server = master_address == 0;
		address_server.hostname = "127.0.0.1";
		address_server.port = Network_port;

		socket_server = socket(AF_INET, SOCK_STREAM, 0);
		if (socket_server < 0)
		{
			printf("Failed to open server socket");
			return false;
		}

		sockaddr_in addr_server = { 0 };
		addr_server.sin_family = AF_INET;
		if (is_server)
		{
			addr_server.sin_addr.s_addr = INADDR_ANY;
		}
		else
		{
			address_master = *master_address;
			// resolve master server hostname
			hostent* server = gethostbyname(address_master.hostname.c_str());
			if (!server)
			{
				printf("Failed to resolve host: %s", address_master.hostname.c_str());
				close(socket_server);
				return false;
			}
			bcopy(server->h_addr, (char*)&addr_server.sin_addr.s_addr, (size_t)server->h_length);
		}
		addr_server.sin_port = htons(address_server.port);

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
		else
		{
			auto connect_result =
				connect(socket_server, (sockaddr*)&addr_server, sizeof(addr_server));
			if (connect_result < 0)
			{
				printf("Failed to connect to master server");
				close(socket_server);
				return false;
			}
			is_connected_to_master = true;
		}

		if (is_server)
		{
			printf("Server started on %s\n", address_server.to_string().c_str());
		}
		else
		{
			printf("Connected to server %s\n", master_address->to_string().c_str());
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

	void accept_thread()
	{
		assert(state == State::Started);

		while (!terminated)
		{
			sockaddr_in addr_client = { 0 };
			uint32 addr_client_size = sizeof(addr_client);

			listen(socket_server, 5);
			Socket socket_client =
				accept(socket_server, (sockaddr*)&addr_client, &addr_client_size);

			if (socket_client < 0)
			{
				printf("Failed to accept client\n");
				return;
			}

			{
				std::lock_guard<std::mutex> _(shared.mutex);

				char buffer[64] = { 0 };
				Address address;
				address.hostname = inet_ntop(AF_INET, &addr_client.sin_addr, buffer, INET_ADDRSTRLEN);
				address.port = ntohs(addr_client.sin_port);
				shared.addresses.push_back(address);
				shared.sockets.push_back(socket_client);
				std::thread* pthread = new std::thread([&] {this->listen_thread(shared.listen_count); });
				shared.threads.push_back(pthread);
				printf("Connection #%s from %s\n", std::to_string(shared.listen_count).c_str(), address.to_string().c_str());
				++shared.listen_count;
			}
		}
	}

	// optional address filter
	bool send(Address* address, std::string message)
	{
		bool ok = true;
		if (is_connected_to_master)
		{
			if (address && !(address_master == *address)) return false;

			ok = ok && send(socket_server, message);
			return ok;
		}

		{
			std::vector<Socket> sockets;
			{
				std::lock_guard<std::mutex> _(shared.mutex);

				for (int32 i = 0; i < shared.sockets.size(); ++i)
				{
					if (address && !(shared.addresses[i] == *address)) continue;
					sockets.push_back(shared.sockets[i]);
				}
			}
			for (auto socket : sockets)
			{
				ok = ok && send(socket, message);
			}
			ok = ok && sockets.size() > 0;
		}
		return ok;
	}

	std::string get_clients() const
	{
		std::string result;
		{
			std::lock_guard<std::mutex> _(shared.mutex);

			for (int32 i = 0; i < shared.sockets.size(); ++i)
			{
				if (shared.sockets[i] < 0) continue;
				result += "#" + std::to_string(i) + " at " + shared.addresses[i].to_string() + "\n";
			}
		}
		return result;
	}

	bool kick_client(int32 slot)
	{
		{
			std::lock_guard<std::mutex> _(shared.mutex);

			if (slot < 0 || slot >= shared.sockets.size()) return false;
			Socket socket = shared.sockets[slot];
			if (socket < 0) return false;

			shutdown(socket, 2);
			// close(socket);

			shared.sockets[slot] = -1;

			return true;
		}

		return false;
	}

	bool read_server(Message& out_message)
	{
		out_message.length = 0;
		int32 n = readn(socket_server, (char*)&out_message.length, sizeof(out_message.length), sizeof(int32));
		if (out_message.length <= 0)
		{
			printf("Incorrect message length\n");
			n = -1;
		}
		if (out_message.length > Message_size_limit)
		{
			printf("Attempt to overflow buffer\n");
			n = -1;
		}
		if (n <= 0) return false;

		n = readn(socket_server, out_message.message, sizeof(out_message.message), out_message.length);
		if (n <= 0) return false;

		return true;
	}

private:
	bool send(Socket socket, std::string in_message)
	{
		Message message;
		message.length = in_message.size();
		bcopy(in_message.c_str(), message.message, message.length);
		int32 n = write(socket, (char*)&message, sizeof(int32) + message.length);

		return n >= 0;
	}

	void listen_thread(int32 offset)
	{
		Socket socket = shared.sockets[offset];
		std::string client_name = std::string("#" + std::to_string(offset));

		while (!terminated)
		{
			int32 length{ 0 };
			int32 n = readn(socket, (char*)&length, sizeof(length), sizeof(int32));
			if (length > Message_size_limit)
			{
				printf("Attempt to overflow buffer\n");
				n = -1;
			}
			if (n <= 0) break;

			printf("Message of length %s incoming, reading\n", std::to_string(length).c_str());

			Message message;
			message.length = length;
			n = readn(socket, message.message, sizeof(message.message), length);
			if (n <= 0) break;

			printf("Received from client %s: %s\n", client_name.c_str(), message.message);

			if (!send(socket, (std::string("Received your message: ") + message.message)))
			{
				printf("Could not write to client %s\n", client_name.c_str());
				break;
			}
		}
		printf("Terminating client connection %s\n", client_name.c_str());


		{
			std::lock_guard<std::mutex> _(shared.mutex);

			shutdown(socket, 2);
			close(socket);
			shared.sockets[offset] = -1;
		}
		return;
	}

	int32 readn(Socket socket, char* buf, int32 buflen, int32 count)
	{
		bzero(buf, buflen);
		int32 n{ 0 };

		while (n < count)
		{
			int32 read_count = read(socket, buf + n, count - n);
			if (read_count == 0)
			{
				printf("Connection terminated by remote user\n");
				return -1;
			}
			else if (read_count < 0)
			{
				printf("Failed to read from server\n");
				return -1;
			}

			n += read_count;
		}

		return n;
	}

	Address address_server;
	Socket socket_server{ 0 };

	Address address_master;
	bool is_connected_to_master{ false };

	bool terminated{ false };

	enum class State
	{
		None,
		Started,
		Terminated
	};
	State state{ State::None };
};
