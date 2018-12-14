#include "SafeSocket.h"
#include "ObjectClass.h"
#include "AttributeHelper.h"

#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

void _add(SafeSocket& client);
void _find(SafeSocket& client);
void _delete(SafeSocket& client);
void _quit(SafeSocket& client);

char terminalBuffer[256] = { '\0' };

void readFromTerminal() {
	bzero(terminalBuffer, sizeof(terminalBuffer));
	fgets(terminalBuffer, sizeof(terminalBuffer), stdin);
	int len = strlen(terminalBuffer);
	if (len > 0 && terminalBuffer[len - 1] == '\n')
		terminalBuffer[len - 1] = 0;
}

void readServerResponse(SafeSocket& client) {
	char* response = nullptr;

	try {
		do {
			response = client.readData();
		} while (response == nullptr);
	} catch (std::exception const& e) {
		_quit(client);
	}

	printf("%s\r\n", response);

	free(response);

	getchar();
}

void sendRequest(SafeSocket& client, char* request) {
	try {
		client.sendData(request);
	}
	catch (std::exception const& e) {
		free(request);
		_quit(client);
	}

	free(request);
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		printf("usage: server-name port\r\n");
		return 1;
	}

	SafeSocket client = SafeSocket(argv[1], argv[2]);

	while (true) {
		system("CLS");
		printf("Actions:\r\n");
		printf("1. Add\r\n");
		printf("2. Find\r\n");
		printf("3. Delete\r\n");
		printf("4. Quit\r\n");

		readFromTerminal();
		if (strcmp(terminalBuffer, "1") == 0) {
			_add(client);
		} else if (strcmp(terminalBuffer, "2") == 0) {
			_find(client);
		} else if (strcmp(terminalBuffer, "3") == 0) {
			_delete(client);
		} else if (strcmp(terminalBuffer, "4") == 0) {
			break;
		}
	}

	_quit(client);
}

void _add(SafeSocket& client) {
	while (true) {
		system("CLS");
		printf("Object class:\r\n");
		printf("1. PosixAccount\r\n");
		printf("2. Device\r\n");
		printf("3. PosixGroup\r\n");
		printf("4. Resource\r\n");
		printf("___________\r\n");
		printf("5. Back\r\n");

		readFromTerminal();
		ObjectClass* object = nullptr;
		if (strcmp(terminalBuffer, "1") == 0) {
			object = new ObjectClass(ObjectClassType::posixAccount);
		} else if (strcmp(terminalBuffer, "2") == 0) {
			object = new ObjectClass(ObjectClassType::device);
		} else if (strcmp(terminalBuffer, "3") == 0) {
			object = new ObjectClass(ObjectClassType::posixGroup);
		} else if (strcmp(terminalBuffer, "4") == 0) {
			object = new ObjectClass(ObjectClassType::resource);
		} else if (strcmp(terminalBuffer, "5") == 0) {
			return;
		} else {
			continue;
		}

		printf("Enter the path:\r\n");
		readFromTerminal();
		char* path = _strdup(terminalBuffer);

		std::vector<Attribute> attributes = object->getAttributes();
		for (int index = 0; index < attributes.size();) {
			Attribute attribute = attributes[index];
			printf("%s:\r\n", AttributeHelper::namedAttribute(attribute).c_str());
			readFromTerminal();
			char* newLinePosition;
			if ((newLinePosition = strchr(terminalBuffer, '\n')) != nullptr) {
				*newLinePosition = '\0';
			}
			if (object->setAttribute(attribute, terminalBuffer)) {
				index++;
			} else {
				printf("Wrong format!\r\n");
			}
		}

		char* serializedObject = ObjectClass::serialize(*object);
		delete object;

		int requestSize = 4 + strlen(path) + 1 + strlen(serializedObject) + 1 + 1;
		char* request = (char*)malloc(requestSize * sizeof(char));
		sprintf_s(request, requestSize, "add?%s&%s;", path, serializedObject);
		
		free(path);
		free(serializedObject);

		sendRequest(client, request);

		readServerResponse(client);

		return;
	}
}

void _find(SafeSocket& client) {
	printf("Enter the name:\r\n");
	readFromTerminal();

	int requestSize = 5 + strlen(terminalBuffer) + 1 + 1;
	char* request = (char*)malloc(requestSize);
	sprintf_s(request, requestSize, "find?%s;", terminalBuffer);
	
	sendRequest(client, request);

	readServerResponse(client);
}

void _delete(SafeSocket& client) {
	printf("Enter the path:\r\n");
	readFromTerminal();

	int requestSize = 7 + strlen(terminalBuffer) + 1 + 1;
	char* request = (char*)malloc(requestSize);
	sprintf_s(request, requestSize, "delete?%s;", terminalBuffer);

	sendRequest(client, request);

	readServerResponse(client);
}

void _quit(SafeSocket& client) {
	client.close();

	printf("Logged off\r\n");
	getchar();

	exit(0);
}
