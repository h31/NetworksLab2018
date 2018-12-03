#include "ObjectClass.h"
#include "AttributeHelper.h"

bool ObjectClass::setAttribute(Attribute attribute, char* value) {
	return setAttributeForList(requiredAttributes, attribute, value) || setAttributeForList(optionalAttributes, attribute, value);
}

bool ObjectClass::setAttributeForList(LinkedList<AttributePair>& list, Attribute attribute, char* value) {
	if (list.count() == 0) {
		return false;
	}

	auto setAttribute = [attribute, value](AttributePair pair) -> bool {
		if (!AttributeHelper::isValid(attribute, value)) {
			return false;
		}

		pair.value = value;
		return true;
	};

	Iterator<AttributePair> iterator = list.iterator();
	AttributePair current = iterator.value();
	while (iterator.hasNext()) {
		if (current.attribute == attribute) {
			return setAttribute(current);
		}

		current = iterator.next();
	}

	if (current.attribute == attribute) {
		return setAttribute(current);
	}
}

bool ObjectClass::isReady() {
	if (requiredAttributes.count() == 0) {
		return true;
	}
	
	Iterator<AttributePair> requiredIterator = requiredAttributes.iterator();
	AttributePair current = requiredIterator.value();
	while (requiredIterator.hasNext()) {
		if (current.value == nullptr) {
			return false;
		}
		current = requiredIterator.next();
	}
	return current.value != nullptr;
}

ObjectClassPosixAccount::ObjectClassPosixAccount() {
	requiredAttributes.add(AttributePair(cn));
	requiredAttributes.add(AttributePair(uid));
	requiredAttributes.add(AttributePair(uidNumber));
	requiredAttributes.add(AttributePair(gidNumber));
	requiredAttributes.add(AttributePair(homeDirectory));

	optionalAttributes.add(AttributePair(loginShell));
	optionalAttributes.add(AttributePair(gecos));
	optionalAttributes.add(AttributePair(description));
	optionalAttributes.add(AttributePair(authPassword));
}

ObjectClassDevice::ObjectClassDevice() {
	requiredAttributes.add(AttributePair(cn));

	optionalAttributes.add(AttributePair(description));
	optionalAttributes.add(AttributePair(i));
	optionalAttributes.add(AttributePair(networkAddress));
	optionalAttributes.add(AttributePair(ou));
	optionalAttributes.add(AttributePair(o));
	optionalAttributes.add(AttributePair(owner));
	optionalAttributes.add(AttributePair(seeAlso));
	optionalAttributes.add(AttributePair(serialNumber));
	optionalAttributes.add(AttributePair(svcType));
	optionalAttributes.add(AttributePair(svcTypeID));
	optionalAttributes.add(AttributePair(svcInfo));
}

ObjectClassPosixGroup::ObjectClassPosixGroup() {
	requiredAttributes.add(AttributePair(gidNumber));

	optionalAttributes.add(AttributePair(authPassword));
	optionalAttributes.add(AttributePair(userPassword));
	optionalAttributes.add(AttributePair(memberUid));
	optionalAttributes.add(AttributePair(description));
}

ObjectClassResource::ObjectClassResource() {
	requiredAttributes.add(AttributePair(cn));

	optionalAttributes.add(AttributePair(hostResourceName));
	optionalAttributes.add(AttributePair(localityName));
	optionalAttributes.add(AttributePair(organization));
	optionalAttributes.add(AttributePair(organizationalUnit));
	optionalAttributes.add(AttributePair(seeAlso));
	optionalAttributes.add(AttributePair(uses));
}