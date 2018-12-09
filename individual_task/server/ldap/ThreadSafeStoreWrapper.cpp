#include "ThreadSafeStoreWrapper.h"

class ScopeLock {
private:
	std::mutex& mutex;
public:
	ScopeLock(std::mutex& mutex) : mutex(mutex) {
		mutex.lock();
	}
	~ScopeLock() {
		mutex.unlock();
	}
};

ThreadSafeStoreWrapper::ThreadSafeStoreWrapper(Store& store) : store(store) {}

void ThreadSafeStoreWrapper::addRecord(const char* name, const char* data) {
	ScopeLock lock(accessMutex);
	store.addRecord(name, data);
}

void ThreadSafeStoreWrapper::deleteRecord(const char* name) {
	ScopeLock lock(accessMutex);
	store.deleteRecord(name);
}

const char* ThreadSafeStoreWrapper::getRecord(const char* name) {
	ScopeLock lock(accessMutex);
	return store.getRecord(name);
}
