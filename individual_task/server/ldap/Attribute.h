#ifndef ATTRIBUTE_H
#define ATTRIBUTE_H

enum Attribute {
	authPassword,  // string
	cn, // string
	description, // string
	gecos, // string list
	gidNumber, // integer
	homeDirectory, // path
	hostResourceName,// string
	i, // string
	localityName, // string
	loginShell, // path
	memberUid, // integer
	networkAddress, // ip
	o, // string
	organization, // string
	organizationalUnit, // string
	ou,
	owner,
	seeAlso,
	serialNumber,
	svcInfo,
	svcType,
	svcTypeID,
	uid,
	uidNumber, // integer
	userPassword,
	uses
};

#endif