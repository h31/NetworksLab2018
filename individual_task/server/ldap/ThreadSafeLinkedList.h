#ifndef THREAD_SAFE_LINKED_LIST_H
#define THREAD_SAFE_LINKED_LIST_H

#include "LinkedList.h"
#include "ScopeLock.h"

template<typename T> class ThreadSafeLinkedList : public LinkedList<T> {
private:
	std::mutex barrierMutex;

public:
	void add(T value) {
		ScopeLock lock(barrierMutex);
		LinkedList<T>::add(value);
	}

	void forEach(std::function<void(T)> lambda) {
		ScopeLock lock(barrierMutex);
		LinkedList<T>::forEach(lambda);
	}

	void remove(T value) {
		ScopeLock lock(barrierMutex);
		LinkedList<T>::remove(value);
	}

	int count() {
		ScopeLock lock(barrierMutex);
		return LinkedList<T>::counter;
	}
};

#endif