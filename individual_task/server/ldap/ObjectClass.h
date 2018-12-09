#ifndef OBJECT_CLASS_H
#define OBJECT_CLASS_H

#include "Attribute.h"
#include "ObjectClassType.h"
#include <vector>

class AttributePair {
public:
	AttributePair(Attribute attribute, bool required) : attribute(attribute), isRequired(required) { }
	const Attribute attribute;
	std::string value;
	const bool isRequired;
};

class ObjectClass {
private:
	std::vector<Attribute> attributes;
	std::vector<AttributePair> values;
	ObjectClassType type;
public:
	ObjectClass(ObjectClassType type);
	bool setAttribute(Attribute attribute, const char* value);
	std::vector<Attribute> getAttributes();
	bool isReady();
	const char* description();
	static const char* serialize(ObjectClass object);
	static ObjectClass deserialize(const char* data);
};

#endif
