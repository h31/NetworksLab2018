#include "Store.h"

class DirectoryTreeStore : public Store {
private:
	const char* prefix;
	char* mallocPathWithPrefix(const char* name);

public:
	DirectoryTreeStore();
	bool addRecord(const char* name, const char* data);
	bool deleteRecord(const char* name);
	const char* getRecord(const char* name);
};
