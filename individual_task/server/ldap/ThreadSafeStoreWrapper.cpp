#include "ThreadSafeStoreWrapper.h"

ThreadSafeStoreWrapper::ThreadSafeStoreWrapper(Store& store) : store(store) {}

void ThreadSafeStoreWrapper::addRecord(const char* name, const char* data) {
	accessMutex.lock();
	try {
		store.addRecord(name, data);
		accessMutex.unlock();
	} catch (const char* error) {
		accessMutex.unlock();
		throw;
	}
}

void ThreadSafeStoreWrapper::deleteRecord(const char* name) {
	accessMutex.lock();
	try {
		store.deleteRecord(name);
		accessMutex.unlock();
	}
	catch (const char* error) {
		accessMutex.unlock();
		throw;
	}
}

const char* ThreadSafeStoreWrapper::getRecord(const char* name) {
	accessMutex.lock();
	const char* searchResult = store.getRecord(name);
	accessMutex.unlock();
	return searchResult;
}
