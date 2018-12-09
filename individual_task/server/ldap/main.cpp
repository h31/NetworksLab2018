#include <iostream>
#include "DirectoryTreeStore.h"
#include "ThreadSafeStoreWrapper.h"
#include "DirectoryUtil.h"
#include "ThreadSafeLinkedList.h"
#include "ObjectClass.h"

int main() {
	//	Store* store = new DirectoryTreeStore();
	//	Store* threadSafeStore = new ThreadSafeStore(*store);
	DirectoryTreeStore* treeStore = new DirectoryTreeStore();
	ThreadSafeStoreWrapper* threadSafeStore = new ThreadSafeStoreWrapper(*treeStore);
	try {
		threadSafeStore->addRecord("p1\\p2\\p3\\p4\\file.txt", "456");
		const char* result = threadSafeStore->getRecord("fi");
		if (result) {
			std::cout << result << "\r\n";
		}
		std::cout << threadSafeStore->getRecord("lol") << "\r\n";
		ObjectClass account(posixAccount);
		std::cout << account.setAttribute(cn, "123") << "\r\n";
		std::cout << account.setAttribute(uid, "123") << "\r\n\r\n";
		std::cout << ObjectClass::serialize(account) << "\r\n";
		threadSafeStore->addRecord("lol.txt", account.description());
		ObjectClass::deserialize("0\n1:123\n18:456\n19:789\n4:012\n5:\\add\\\n");
	} catch (const char* error) {
		std::cout << error << "\r\n";
	}
	//std::cout << threadSafeStore->deleteRecord("p1\\p2\\p3\\p4\\file.txt") << "\r\n";

	{
		ThreadSafeLinkedList<int> list;
		list.add(1);
		list.add(2);
		list.add(3);
		list.add(4);
		list.add(5);
		list.add(6);
		auto funct = [](int lol) {
			std::cout << lol * lol << "\r\n";
		};
		list.forEach(funct);
	}

	//std::cout << threadSafeStore->addRecord("123", "456") << "\r\n";
	//std::cout << threadSafeStore->addRecord("123", "456") << "\r\n";
	//threadSafeStore->findRecord("111111111111");
	getchar();
	return 0;
}
