#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

enum Attribute {
	authPassword,  // string
	cn, // string
	description, // string
	gecos, // string
	gidNumber, // integer
	homeDirectory, // path
	hostResourceName, // string
	i, // string
	localityName, // string
	loginShell, // path
	memberUid, // integer
	networkAddress, // ip
	o, // string
	owner, // string
	serialNumber, // string
	svcInfo, // string
	svcType, // string
	svcTypeID, // integer
	uid, // string
	uidNumber, // integer
	userPassword, // string
	uses // string
};

#endif