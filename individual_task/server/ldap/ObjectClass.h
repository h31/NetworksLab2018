#include "Attribute.h"
#include "LinkedList.h"

class AttributePair {
public:
	const Attribute attribute;
	const char* value = nullptr;
	const bool isRequired;
	AttributePair(Attribute attribute, bool required) : attribute(attribute), isRequired(required) { }
};

class ObjectClass {
private:
	Iterator<AttributePair*>* iterator = nullptr;
	Iterator<AttributePair*>* getIterator();
protected:
	LinkedList<AttributePair*> attributes;
public:
	//ObjectClass();
	bool setAttribute(const char* value);
	const char* currentAttributeName();
	bool isReady();
	~ObjectClass();
};

class ObjectClassPosixAccount : public ObjectClass {
public:
	ObjectClassPosixAccount();
};

class ObjectClassDevice : public ObjectClass {
public:
	ObjectClassDevice();
};

class ObjectClassPosixGroup : public ObjectClass {
public:
	ObjectClassPosixGroup();
};

class ObjectClassResource : public ObjectClass {
public:
	ObjectClassResource();
};