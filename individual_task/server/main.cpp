#undef _DEBUG
#include "Server.h"

int main() {

    init_log();
	Server server(5001);

	return 0;
}


