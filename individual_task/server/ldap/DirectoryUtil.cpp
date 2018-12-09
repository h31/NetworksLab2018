#include "DirectoryUtil.h"
#include <direct.h>
#include <Windows.h>

bool DirectoryUtil::makePath(std::string path) {
	size_t backslashIndex;
	for (backslashIndex = path.find('\\', 1); backslashIndex != std::string::npos; backslashIndex = path.find('\\', backslashIndex + 1)) {
		path[backslashIndex] = '\0';
		if (_mkdir(path.c_str()) == ENOENT) {
			path[backslashIndex] = '\\';
			return false;
		}

		path[backslashIndex] = '\\';
	}

	return true;
}
