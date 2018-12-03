#include "FileUtil.h"
#include <iostream>

bool FileUtil::createAndWriteToFile(const char* filename, const char* data) {
	FILE* newFile;

	if (fopen_s(&newFile, filename, "wx") != 0) {
		return false;
	}

	fprintf(newFile, data);
	fclose(newFile);

	return true;
}

char* FileUtil::readFile(const char* filename) {
	return nullptr;
}

bool FileUtil::deleteFile(const char* filename) {
	return remove(filename) == 0;
}
