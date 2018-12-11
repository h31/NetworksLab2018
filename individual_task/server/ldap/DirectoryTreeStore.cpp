#include "DirectoryTreeStore.h"
#include "DirectoryUtil.h"
#include "FileUtil.h"

DirectoryTreeStore::DirectoryTreeStore() {
	prefix = std::string("store");
}

void DirectoryTreeStore::addRecord(const char* name, const char* data) {
	std::string path = createPathWithPrefix(name);

	DirectoryUtil::makePath(path);

	FileUtil::createAndWriteToFile(path, data);
}

void DirectoryTreeStore::deleteRecord(const char* name) {
	std::string path = createPathWithPrefix(name);

	FileUtil::deleteFile(path);
}

char* DirectoryTreeStore::getRecord(const char* name) {
	std::string fullpath = FileUtil::findFile(prefix, name);

	if (fullpath.empty()) {
		throw "File does not exist";
	}

	std::string readData = FileUtil::readFile(fullpath);

	if (readData.empty()) {
		throw "Internal server error";
	}

	fullpath.erase(0, fullpath.find('\\'));

	std::string response = fullpath + "\r\n" + readData;

	return _strdup(response.c_str());
}

std::string DirectoryTreeStore::createPathWithPrefix(std::string name) {
	if (name.empty()) {
		throw "Name could not be empty";
	}

	return (name.find("\\") == 0) ? prefix + name : prefix + "\\" + name;
}
