#include "Attribute.h"

class AttributeHelper {
public:
	static bool isValid(Attribute attribute, const char* value, bool required);
	static const char* namedAttribute(Attribute attribute);
};
