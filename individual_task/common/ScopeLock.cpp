#include "ScopeLock.h"

ScopeLock::ScopeLock(std::mutex& mutex) : mutex(mutex) {
	mutex.lock();
}

ScopeLock::~ScopeLock() {
	mutex.unlock();
}