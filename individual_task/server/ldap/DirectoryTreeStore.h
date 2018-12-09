#include "Store.h"
#include <string>

class DirectoryTreeStore : public Store {
private:
	std::string prefix;
	std::string createPathWithPrefix(std::string name);

public:
	DirectoryTreeStore();
	void addRecord(const char* name, const char* data);
	void deleteRecord(const char* name);
	const char* getRecord(const char* name);
};
