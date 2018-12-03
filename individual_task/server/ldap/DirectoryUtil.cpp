#include "DirectoryUtil.h"
#include <direct.h>
#include <stdio.h>
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

char* DirectoryUtil::findFile(const char* currentSearchFolder, const char* filename) {
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
		} else if ((object.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
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
