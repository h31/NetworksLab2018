#include "logging.h"

void log_close() {
	int res;
	res = fclose(file_to_log);
	if(res != 0) {
		printf("ERROR. Can not close log file with error code = %d", res);
	}
}
