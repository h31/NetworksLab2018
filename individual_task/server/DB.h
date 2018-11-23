#pragma once

#include <libpq-fe.h>
#include "TableString.h"

class DB {
public:
    DB();
    ~DB();
    const char* findUser(char* login, char* pass, string& name, bool& isClient);
    int saveData(const char *clientId, string month, string year, const char *metersData);
    int getData(const char *clientId, const char *monthToShow, string *result);
    const char * getClientItByName(const char *login);
    int getDefaulters(const char *month, const char *year, string *result);
private:
    PGconn* conn = NULL;
    PGresult* res = NULL;
};

