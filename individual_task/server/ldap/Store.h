#ifndef STORE_H
#define STORE_H

class Store {
public:
	virtual void addRecord(const char* name, const char* data) = 0;
	virtual void deleteRecord(const char* name) = 0;
	virtual char* getRecord(const char* name) = 0;
};

#endif
