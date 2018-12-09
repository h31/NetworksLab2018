#include "DirectoryTreeStore.h"
#include "DirectoryUtil.h"
#include "FileUtil.h"

DirectoryTreeStore::DirectoryTreeStore() {
	prefix = std::string("store");
}

bool DirectoryTreeStore::addRecord(const char* name, const char* data) {
	std::string path = createPathWithPrefix(name);

	if (!DirectoryUtil::makePath(path)) {
		return false;
	}

	return FileUtil::createAndWriteToFile(path, data);
}

bool DirectoryTreeStore::deleteRecord(const char* name) {
	std::string path = createPathWithPrefix(name);
	return FileUtil::deleteFile(path);
}

const char* DirectoryTreeStore::getRecord(const char* name) {
	std::string fullpath = FileUtil::findFile(prefix, name);

	if (fullpath.empty()) {
		return nullptr;
	}

	std::string readData = FileUtil::readFile(fullpath);

	if (readData.empty()) {
		return nullptr;
	}

	fullpath.erase(0, fullpath.find('\\'));

	std::string response = fullpath + "\r\n" + readData;

	return _strdup(response.c_str());
}

std::string DirectoryTreeStore::createPathWithPrefix(std::string name) {
	return prefix + "\\" + name;
}
