#include "ThreadSafeStoreWrapper.h"

ThreadSafeStoreWrapper::ThreadSafeStoreWrapper(Store& store) : store(store) {}

bool ThreadSafeStoreWrapper::addRecord(const char* name, const char* data) {
	accessMutex.lock();
	bool additionResult = store.addRecord(name, data);
	accessMutex.unlock();
	return additionResult;
}

bool ThreadSafeStoreWrapper::deleteRecord(const char* name) {
	accessMutex.lock();
	bool deletionResult = store.deleteRecord(name);
	accessMutex.unlock();
	return deletionResult;
}

const char* ThreadSafeStoreWrapper::getRecord(const char* name) {
	accessMutex.lock();
	const char* searchResult = store.getRecord(name);
	accessMutex.unlock();
	return searchResult;
}
