#include "FileUtil.h"
#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>

void FileUtil::createAndWriteToFile(std::string filename, std::string data) {
	std::ifstream existenceCheck(filename);
	if (!existenceCheck.fail()) {
		throw "File already exists";
	}
	existenceCheck.close();

	std::ofstream outputFile(filename);

	if (!outputFile.is_open()) {
		throw "Internal server error";
	}

	outputFile << data;

	outputFile.close();
}

std::string FileUtil::readFile(std::string filename) {
	std::ifstream file(filename);

	if (!file.is_open()) {
		return std::string();
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string FileUtil::findFile(std::string currentSearchFolder, std::string filename) {
	char currentSearchFolderWithWildcard[MAX_PATH];
	sprintf_s(currentSearchFolderWithWildcard, MAX_PATH, "%s\\*", currentSearchFolder.c_str());

	WIN32_FIND_DATA object;
	HANDLE handle = FindFirstFile(currentSearchFolderWithWildcard, &object);
	if (handle == INVALID_HANDLE_VALUE) {
		return std::string();
	}

	do {
		if (strcmp(object.cFileName, ".") == 0 || strcmp(object.cFileName, "..") == 0) {
			continue;
		}

		char currentObjectPath[MAX_PATH];
		sprintf_s(currentObjectPath, MAX_PATH, "%s\\%s", currentSearchFolder.c_str(), object.cFileName);

		if (strstr(object.cFileName, filename.c_str()) != 0 && (object.dwFileAttributes ^ FILE_ATTRIBUTE_DIRECTORY) != 0) {
			FindClose(handle);
			return std::string(currentObjectPath);
		} else if ((object.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			std::string subfolderSearchResult = findFile(std::string(currentObjectPath), filename);
			if (subfolderSearchResult.empty()) {
				continue;
			}
			FindClose(handle);
			return subfolderSearchResult;
		}
	} while (FindNextFile(handle, &object));

	FindClose(handle);

	return std::string();
}

void FileUtil::deleteFile(std::string filename) {
	if (remove(filename.c_str()) != 0) {
		throw "Internal server error";
	}
}
