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
	char* fullpath = DirectoryUtil::findFile(prefix, name);

	if (fullpath == nullptr) {
		return nullptr;
	}

	char* trimmedStorePath = _strdup(strchr(fullpath, '\\'));
	free(fullpath);

	return trimmedStorePath;
}

char* DirectoryTreeStore::mallocPathWithPrefix(const char* name) {
	int pathLength = (strlen(prefix) + 1 + strlen(name) + 1) * sizeof(char);
	char* path = (char*)malloc(pathLength);
	sprintf_s(path, pathLength, "%s\\%s", prefix, name);
	return path;
}
