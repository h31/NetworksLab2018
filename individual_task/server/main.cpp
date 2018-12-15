//#undef _DEBUG
#define _DEBUG
#include "Server.h"

int main() {

    init_log();
	Server* server = new Server(5001);

	delete server;
    BOOST_LOG_TRIVIAL(info) << "Сервер успешно завершил свою работу";

	return 0;
}


