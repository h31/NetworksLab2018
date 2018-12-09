#include "Store.h"
#include <string>

class DirectoryTreeStore : public Store {
private:
	std::string prefix;
	std::string createPathWithPrefix(std::string name);

public:
	DirectoryTreeStore();
	bool addRecord(const char* name, const char* data);
	bool deleteRecord(const char* name);
	const char* getRecord(const char* name);
};
