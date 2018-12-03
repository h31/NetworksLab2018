#include "Attribute.h"
#include "LinkedList.h"

class AttributePair {
public:
	Attribute attribute;
	char* value = nullptr;
	AttributePair(Attribute attribute) : attribute(attribute) { }
};

class ObjectClass {
private:
	bool setAttributeForList(LinkedList<AttributePair>& list, Attribute attribute, char* value);
protected:
	LinkedList<AttributePair> requiredAttributes;
	LinkedList<AttributePair> optionalAttributes;
public:
	bool setAttribute(Attribute attribute, char* value);
	bool isReady();
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