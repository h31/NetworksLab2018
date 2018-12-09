#include "AttributeHelper.h"
#include <regex>

bool AttributeHelper::isValid(Attribute attribute, const char* value, bool required) {
	if (required && (strlen(value) == 0)) {
		return false;
	}

	switch (attribute) {
	case gidNumber: case memberUid: case svcTypeID: case uidNumber:
		return std::regex_match(value, std::regex("\\d+"));

	case homeDirectory: case loginShell:
		return std::regex_match(value, std::regex("([\\\\])([\\w]+[\\\\]?)*"));

	case networkAddress:
		// https://stackoverflow.com/a/25969006
		return std::regex_match(value, std::regex("((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"));

	default:
		return true;
	}
}

std::string AttributeHelper::namedAttribute(Attribute attribute) {
	switch (attribute) {
	case authPassword:		return "authPassword";
	case cn:				return "cn";
	case description:		return "description";
	case gecos:				return "gecos";
	case gidNumber:			return "gidNumber";
	case homeDirectory:		return "homeDirectory";
	case hostResourceName:	return "hostResourceName";
	case i:					return "i";
	case localityName:		return "localityName";
	case loginShell:		return "loginShell";
	case memberUid:			return "memberUid";
	case networkAddress:	return "networkAddress";
	case o:					return "o";
	case owner:				return "owner";
	case serialNumber:		return "serialNumber";
	case svcInfo:			return "svcInfo";
	case svcType:			return "svcType";
	case svcTypeID:			return "svcTypeID";
	case uid:				return "uid";
	case uidNumber:			return "uidNumber";
	case userPassword:		return "userPassword";
	case uses:				return "uses";
	default:				throw;
	}
}