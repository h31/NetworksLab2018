#include <iostream>
#include "DirectoryTreeStore.h"
#include "ThreadSafeStoreWrapper.h"
#include "DirectoryUtil.h"
#include "LinkedList.h"
#include "ObjectClass.h"

class MyData {

};

int main() {
	//	Store* store = new DirectoryTreeStore();
	//	Store* threadSafeStore = new ThreadSafeStore(*store);
	DirectoryTreeStore* treeStore = new DirectoryTreeStore();
	ThreadSafeStoreWrapper* threadSafeStore = new ThreadSafeStoreWrapper(*treeStore);
	std::cout << threadSafeStore->addRecord("p1\\p2\\p3\\p4\\file.txt", "456") << "\r\n";
	//std::cout << threadSafeStore->deleteRecord("p1\\p2\\p3\\p4\\file.txt") << "\r\n";
	const char* result = threadSafeStore->getRecord("super");
	if (result) {
		std::cout << result << "\r\n";
	}

	{
		LinkedList<int> list;
		list.add(1);
		list.add(2);
		list.add(3);
		list.add(4);
		list.add(5);
		list.add(6);

		Iterator<int> iter = list.iterator();
		std::cout << iter.value() << "\r\n";
		while (iter.hasNext()) {
			std::cout << iter.next() << "\r\n";
		}

		list.remove(6);
		list.remove(1);
		list.remove(100);
		list.add(10);

		iter = list.iterator();
		std::cout << iter.value() << "\r\n";
		while (iter.hasNext()) {
			std::cout << iter.next() << "\r\n";
		}
	}

	{
		std::cout << ObjectClassPosixAccount().setAttribute(cn, nullptr) << "\r\n";
	}


//	std::cout << threadSafeStore->addRecord("123", "456") << "\r\n";
	//std::cout << threadSafeStore->addRecord("123", "456") << "\r\n";
	//threadSafeStore->findRecord("111111111111");
	getchar();
	return 0;
}
