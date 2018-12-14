#ifndef SCOPE_LOCK_H
#define SCOPE_LOCK_H

#include <mutex>

class ScopeLock {
private:
	std::mutex& mutex;
public:
	ScopeLock(std::mutex& mutex);
	~ScopeLock();
};

#endif