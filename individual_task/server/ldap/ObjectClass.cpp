#include "ObjectClass.h"
#include "ObjectClassAttributesFactory.h"
#include "AttributeHelper.h"
#include "ObjectClassTypeHelper.h"
#include <algorithm>
#include <sstream>

ObjectClass::ObjectClass(ObjectClassType type) : type(type) {
	this->values = ObjectClassAttributesFactory::create(type);

	for (int index = 0; index < values.size(); ++index) {
		attributes.push_back(values[index].attribute);
	}
}

bool ObjectClass::setAttribute(Attribute attribute, const char* value) {
	std::vector<Attribute>::iterator searchIterator = std::find(attributes.begin(), attributes.end(), attribute);
	if (searchIterator == attributes.end()) {
		return false;
	}

	int index = std::distance(attributes.begin(), searchIterator);
	AttributePair& pair = values[index];
	if (!AttributeHelper::isValid(attribute, value, pair.isRequired)) {
		return false;
	}

	pair.value = std::string(value);

	return true;
}

std::vector<Attribute> ObjectClass::getAttributes() {
	return attributes;
}

bool ObjectClass::isReady() {
	for (int index = 0; index < values.size(); ++index) {
		AttributePair pair = values[index];
		if (pair.isRequired && pair.value.empty()) {
			return false;
		}
	}

	return true;
}

const char* ObjectClass::description() {
	std::stringstream buffer;

	buffer << "objectClass: " << ObjectClassTypeHelper::namedObjectClassType(type) << "\n";
	for (int index = 0; index < values.size(); ++index) {
		AttributePair pair = values[index];
		buffer << AttributeHelper::namedAttribute(pair.attribute) << ": " << pair.value << "\n";
	}

	return _strdup(buffer.str().c_str());
}

const char* ObjectClass::serialize(ObjectClass object) {
	std::stringstream buffer;
	buffer << object.type << "\n";

	std::vector<AttributePair> values = object.values;
	for (int index = 0; index < values.size(); ++index) {
		AttributePair pair = values[index];
		buffer << pair.attribute << ":" << pair.value << "\n";
	}

	return _strdup(buffer.str().c_str());
}

ObjectClass ObjectClass::deserialize(const char* data) {
	std::string buffer(data);
	if (buffer.empty()) {
		throw "Object class is empty";
	}

	int typeIndex = buffer.find("\n");
	if (typeIndex == std::string::npos) {
		throw "Object class type is missing";
	}
	ObjectClassType typeID = ObjectClassType(std::stoi(buffer.substr(0, typeIndex)));
	if (typeID < ObjectClassType::posixAccount || typeID > ObjectClassType::resource) {
		throw "Bad object class type";
	}

	ObjectClass object = ObjectClass(typeID);

	int currentRowIndex;
	for (currentRowIndex = typeIndex + 1; currentRowIndex != std::string::npos; currentRowIndex = buffer.find("\n", currentRowIndex + 1) + 1) {
		if (currentRowIndex >= buffer.size()) {
			break;
		}

		int colonIndex = buffer.find(":", currentRowIndex);
		if (colonIndex == std::string::npos) {
			throw "Bad object class format";
		}
		Attribute attribute = Attribute(std::stoi(buffer.substr(currentRowIndex, colonIndex - currentRowIndex)));
		if (attribute < Attribute::authPassword || attribute > Attribute::uses) {
			throw "Bad object class format";
		}

		int nextRowIndex = buffer.find("\n", colonIndex);
		if (nextRowIndex == std::string::npos) {
			throw "Bad object class format";
		}
		std::string value = buffer.substr(colonIndex + 1, nextRowIndex - colonIndex - 1);
		if (!object.setAttribute(attribute, value.c_str())) {
			throw "Bad object class format";
		}
	}

	if (!object.isReady()) {
		throw "Required attributes are not set";
	}

	return object;
}
