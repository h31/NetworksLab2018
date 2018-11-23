#undef _DEBUG
#include "Client.h"

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "usage %s hostname port\n", argv[0]);
		exit(0);
	}

	init_log();

	try {
        Client client(argv[1], argv[2]);
        client.handler();
    } catch (const runtime_error& ex){
        BOOST_LOG_TRIVIAL(error) << ex.what();
        BOOST_LOG_TRIVIAL(error) << "Server was unexpectedly disconnected";
    }

	return 0;
}
