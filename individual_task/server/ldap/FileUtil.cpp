#include "FileUtil.h"
#include <Windows.h>
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
	FILE* file;
	
	if (fopen_s(&file, filename, "rb") != 0) {
		return nullptr;
	}

	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	char* buffer = (char*)malloc((length + 1) * sizeof(char));

	fseek(file, 0, SEEK_SET);
	long readSize = fread(buffer, sizeof(char), length, file);

	if (readSize != length) {
		free(buffer);
		return nullptr;
	}

	buffer[length] = '\0';

	fclose(file);

	return buffer;
}

char* FileUtil::findFile(const char* currentSearchFolder, const char* filename) {
	char currentSearchFolderWithWildcard[MAX_PATH];
	sprintf_s(currentSearchFolderWithWildcard, MAX_PATH, "%s\\*", currentSearchFolder);

	WIN32_FIND_DATA object;
	HANDLE handle = FindFirstFile(currentSearchFolderWithWildcard, &object);
	if (handle == INVALID_HANDLE_VALUE) {
		return nullptr;
	}

	do {
		if (strcmp(object.cFileName, ".") == 0 || strcmp(object.cFileName, "..") == 0) {
			continue;
		}

		char* currentObjectPath = (char*)malloc(MAX_PATH * sizeof(char));
		sprintf_s(currentObjectPath, MAX_PATH, "%s\\%s", currentSearchFolder, object.cFileName);

		if (strstr(object.cFileName, filename) != 0 && (object.dwFileAttributes ^ FILE_ATTRIBUTE_DIRECTORY) != 0) {
			FindClose(handle);
			return currentObjectPath;
		}
		else if ((object.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
			char* subfolderSearchResult = findFile(currentObjectPath, filename);
			free(currentObjectPath);
			if (subfolderSearchResult == nullptr) {
				continue;
			}
			FindClose(handle);
			return subfolderSearchResult;
		}
	} while (FindNextFile(handle, &object));

	FindClose(handle);

	return nullptr;
}

bool FileUtil::deleteFile(const char* filename) {
	return remove(filename) == 0;
}
