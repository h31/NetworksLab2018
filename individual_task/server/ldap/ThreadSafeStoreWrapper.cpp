#include "ThreadSafeStoreWrapper.h"
#include "ScopeLock.h"

ThreadSafeStoreWrapper::ThreadSafeStoreWrapper(Store& store) : store(store) {}

void ThreadSafeStoreWrapper::addRecord(const char* name, const char* data) {
	ScopeLock lock(barrierMutex);
	store.addRecord(name, data);
}

void ThreadSafeStoreWrapper::deleteRecord(const char* name) {
	ScopeLock lock(barrierMutex);
	store.deleteRecord(name);
}

const char* ThreadSafeStoreWrapper::getRecord(const char* name) {
	ScopeLock lock(barrierMutex);
	return store.getRecord(name);
}
