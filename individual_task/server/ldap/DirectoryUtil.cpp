#include "DirectoryUtil.h"
#include <direct.h>
#include <Windows.h>

bool DirectoryUtil::makePath(char* path) {
	char* backslashPointer;
	for (backslashPointer = strchr(path + 1, '\\'); backslashPointer; backslashPointer = strchr(backslashPointer + 1, '\\')) {
		*backslashPointer = '\0';

		if (_mkdir(path) == ENOENT) {
			*backslashPointer = '\\';
			return false;
		}

		*backslashPointer = '\\';
	}

	return true;
}
