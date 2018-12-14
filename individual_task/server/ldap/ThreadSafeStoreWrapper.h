#include "Store.h"
#include <mutex>

class ThreadSafeStoreWrapper : public Store {
private:
	Store* store;
	std::mutex barrierMutex;

public:
	ThreadSafeStoreWrapper(Store* store);
	void addRecord(const char* name, const char* data);
	void deleteRecord(const char* name);
	char* getRecord(const char* name);
	~ThreadSafeStoreWrapper();
};
