#include "ObjectClass.h"
#include "AttributeHelper.h"

bool ObjectClass::setAttribute(const char* value) {
	Iterator<AttributePair*>* iterator = getIterator();
	AttributePair* pair = iterator->value();
	
	if (pair == nullptr) {
		return false;
	}

	if (!AttributeHelper::isValid(pair->attribute, value, pair->isRequired)) {
		return false;
	}

	pair->value = value;
	iterator->next();

	return true;
	
	//if (attributes.count() == 0) {
	//	return false;
	//}

	//auto setAttribute = [value](AttributePair* pair) -> bool {
	//	if (!AttributeHelper::isValid(attribute, value, pair->isRequired)) {
	//		return false;
	//	}

	//	pair->value = value;
	//	return true;
	//};

	//Iterator<AttributePair*> iterator = attributes.iterator();
	//AttributePair* current = iterator.value();
	//while (iterator.hasNext()) {
	//	if (current->attribute == attribute) {
	//		return setAttribute(current);
	//	}

	//	current = iterator.next();
	//}

	//if (current->attribute == attribute) {
	//	return setAttribute(current);
	//}

	//return false;
}

Iterator<AttributePair*>* ObjectClass::getIterator() {
	if (iterator == nullptr) {
		iterator = attributes.iterator();
	}
	return iterator;
}

const char* ObjectClass::currentAttributeName() {
	return AttributeHelper::namedAttribute(getIterator()->value()->attribute);
}

bool ObjectClass::isReady() {
	if (attributes.count() == 0) {
		return true;
	}
	
	Iterator<AttributePair*>* iterator = attributes.iterator();
	AttributePair* current = iterator->value();
	while (iterator->hasNext()) {
		if (current->isRequired && current->value == nullptr) {
			return false;
		}
		current = iterator->next();
	}
	return !current->isRequired || current->value != nullptr;
}

ObjectClass::~ObjectClass() {
	if (attributes.count() > 0) {
		Iterator<AttributePair*> iterator = attributes.iterator();
		AttributePair* current = iterator.value();
		while (iterator.hasNext()) {
			delete current;
			current = iterator.next();
		}
		delete current;
	}
}

ObjectClassPosixAccount::ObjectClassPosixAccount() {
	attributes.add(new AttributePair(cn, true));
	attributes.add(new AttributePair(uid, true));
	attributes.add(new AttributePair(uidNumber, true));
	attributes.add(new AttributePair(gidNumber, true));
	attributes.add(new AttributePair(homeDirectory, true));

	attributes.add(new AttributePair(loginShell, false));
	attributes.add(new AttributePair(gecos, false));
	attributes.add(new AttributePair(description, false));
	attributes.add(new AttributePair(authPassword, false));
}

ObjectClassDevice::ObjectClassDevice() {
	attributes.add(new AttributePair(cn, true));

	attributes.add(new AttributePair(description, false));
	attributes.add(new AttributePair(i, false));
	attributes.add(new AttributePair(networkAddress, false));
	attributes.add(new AttributePair(o, false));
	attributes.add(new AttributePair(owner, false));
	attributes.add(new AttributePair(serialNumber, false));
	attributes.add(new AttributePair(svcType, false));
	attributes.add(new AttributePair(svcTypeID, false));
	attributes.add(new AttributePair(svcInfo, false));
}

ObjectClassPosixGroup::ObjectClassPosixGroup() {
	attributes.add(new AttributePair(gidNumber, true));

	attributes.add(new AttributePair(authPassword, false));
	attributes.add(new AttributePair(userPassword, false));
	attributes.add(new AttributePair(memberUid, false));
	attributes.add(new AttributePair(description, false));
}

ObjectClassResource::ObjectClassResource() {
	attributes.add(new AttributePair(cn, true));

	attributes.add(new AttributePair(hostResourceName, false));
	attributes.add(new AttributePair(localityName, false));
	attributes.add(new AttributePair(uses, false));
}