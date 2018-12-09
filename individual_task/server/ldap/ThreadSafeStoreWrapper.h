#include "Store.h"
#include <mutex>

class ThreadSafeStoreWrapper : public Store {
private:
	Store& store;
	std::mutex accessMutex;

public:
	ThreadSafeStoreWrapper(Store& store);
	void addRecord(const char* name, const char* data);
	void deleteRecord(const char* name);
	const char* getRecord(const char* name);
};
