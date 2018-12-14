#include "ObjectClassTypeHelper.h"

std::string ObjectClassTypeHelper::namedObjectClassType(ObjectClassType type) {
	switch (type) {
	case posixAccount:	return "posixAccout";
	case device:		return "device";
	case posixGroup:	return "posixGroup";
	case resource:		return "resource";
	default:			throw;
	}
}