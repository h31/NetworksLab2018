#ifndef LINKED_LIST_H
#define LINKED_LIST_H

template<class T> class Node {
public:
	T value;
	Node* next;
	Node(T value, Node* next) : value(value), next(next) { }
};

template<class T> class Iterator {
private:
	Node<T>* current;

public:
	Iterator(Node<T>* current) : current(current) { }

	T value() {
		return current->value;
	}

	bool hasNext() {
		if (current != nullptr) {
			return current->next != nullptr;
		}
		return false;
	}

	T& next() {
		current = current->next;
		return current->value;
	}
};

template<class T> class LinkedList {
private:
	Node<T>* head;
	Node<T>* tail;
	int counter;

public:
	LinkedList() : counter(0), head(nullptr), tail(nullptr) { }

	void add(T value) {
		Node<T>* newNode = new Node<T>(value, nullptr);

		if (tail != nullptr) {
			tail->next = newNode;
			tail = newNode;
		} else {
			head = newNode;
			tail = newNode;
		}

		counter++;
	}

	void remove(T value) {
		if (head == nullptr) {
			return;
		}

		Node<T>* current = head;
		Node<T>* predecessor = nullptr;

		while (current != nullptr && current->value != value) {
			predecessor = current;
			current = current->next;
		}

		if (current == nullptr) {
			return;
		}

		if (current == head) {
			head = current->next;
		}

		if (current == tail) {
			tail = predecessor;
		}

		if (predecessor != nullptr) {
			predecessor->next = current->next;
		}

		delete current;

		counter--;
	}

	Iterator<T> iterator() {
		return Iterator<T>(head);
	}

	int count() {
		return counter;
	}

	~LinkedList() {
		Node<T>* current = head;
		Node<T>* next = nullptr;

		while (current != nullptr) {
			next = current->next;
			delete current;
			current = next;
		}
	}
};

#endif