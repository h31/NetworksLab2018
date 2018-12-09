#include "DirectoryTreeStore.h"
#include "DirectoryUtil.h"
#include "FileUtil.h"
#include <iostream>

DirectoryTreeStore::DirectoryTreeStore() {
	prefix = "store";
}

bool DirectoryTreeStore::addRecord(const char* name, const char* data) {
	char* path = mallocPathWithPrefix(name);

	if (!DirectoryUtil::makePath(path)) {
		free(path);
		return false;
	}

	bool additionResult = FileUtil::createAndWriteToFile(path, data);

	free(path);

	return additionResult;
}

bool DirectoryTreeStore::deleteRecord(const char* name) {
	char* path = mallocPathWithPrefix(name);
	bool deletionResult = FileUtil::deleteFile(path);
	free(path);
	return deletionResult;
}

const char* DirectoryTreeStore::getRecord(const char* name) {
	char* fullpath = FileUtil::findFile(prefix, name);

	if (fullpath == nullptr) {
		return nullptr;
	}

	char* readData = FileUtil::readFile(fullpath);

	if (readData == nullptr) {
		free(fullpath);
		return nullptr;
	}

	char* trimmedStorePath = _strdup(strchr(fullpath, '\\'));
	free(fullpath);

	int responseSize = (strlen(trimmedStorePath) + 2 + strlen(readData) + 1) * sizeof(char);
	char* response = (char*)malloc(responseSize);
	sprintf_s(response, responseSize, "%s\r\n%s", trimmedStorePath, readData);

	free(trimmedStorePath);
	free(readData);

	return response;
}

char* DirectoryTreeStore::mallocPathWithPrefix(const char* name) {
	int pathLength = (strlen(prefix) + 1 + strlen(name) + 1) * sizeof(char);
	char* path = (char*)malloc(pathLength);
	sprintf_s(path, pathLength, "%s\\%s", prefix, name);
	return path;
}
