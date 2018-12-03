#ifndef STORE_H
#define STORE_H

class Store {
public:
	virtual bool addRecord(const char* name, const char* data) = 0;
	virtual bool deleteRecord(const char* name) = 0;
	virtual const char* getRecord(const char* name) = 0;
};

#endif
