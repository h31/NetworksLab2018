#include "ObjectClassAttributesFactory.h"
#include <vector>

std::vector<AttributePair> ObjectClassAttributesFactory::create(ObjectClassType type) {
	std::vector<AttributePair> attributes;

	switch (type) {
	case posixAccount:
		attributes.push_back(AttributePair(cn, true));
		attributes.push_back(AttributePair(uid, true));
		attributes.push_back(AttributePair(uidNumber, true));
		attributes.push_back(AttributePair(gidNumber, true));
		attributes.push_back(AttributePair(homeDirectory, true));

		attributes.push_back(AttributePair(loginShell, false));
		attributes.push_back(AttributePair(gecos, false));
		attributes.push_back(AttributePair(description, false));
		attributes.push_back(AttributePair(authPassword, false));

		break;

	case device:
		attributes.push_back(AttributePair(cn, true));

		attributes.push_back(AttributePair(description, false));
		attributes.push_back(AttributePair(i, false));
		attributes.push_back(AttributePair(networkAddress, false));
		attributes.push_back(AttributePair(o, false));
		attributes.push_back(AttributePair(owner, false));
		attributes.push_back(AttributePair(serialNumber, false));
		attributes.push_back(AttributePair(svcType, false));
		attributes.push_back(AttributePair(svcTypeID, false));
		attributes.push_back(AttributePair(svcInfo, false));

		break;

	case posixGroup:
		attributes.push_back(AttributePair(gidNumber, true));

		attributes.push_back(AttributePair(authPassword, false));
		attributes.push_back(AttributePair(userPassword, false));
		attributes.push_back(AttributePair(memberUid, false));
		attributes.push_back(AttributePair(description, false));

		break;

	case resource:
		attributes.push_back(AttributePair(cn, true));

		attributes.push_back(AttributePair(hostResourceName, false));
		attributes.push_back(AttributePair(localityName, false));
		attributes.push_back(AttributePair(uses, false));
	}

	return attributes;
}