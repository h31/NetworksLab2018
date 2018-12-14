#include "Server.h"

int main() {
	Server server(5001, 100);
	server.start();

	getchar();
	return 0;
}
