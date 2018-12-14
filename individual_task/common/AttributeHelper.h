#include "Attribute.h"
#include <string>

class AttributeHelper {
public:
	static bool isValid(Attribute attribute, const char* value, bool required);
	static std::string namedAttribute(Attribute attribute);
};
