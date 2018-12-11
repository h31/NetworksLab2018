#include "ObjectClass.h"

class ObjectClassAttributesFactory {
public:
	static std::vector<AttributePair> create(ObjectClassType type);
};