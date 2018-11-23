#pragma once

#include "includes.h"
#include <iomanip>

class TableString {
public:
    explicit TableString(int width = 14, char delimiter = '|');
    ~TableString();
    string getString();

    friend TableString& operator<<(TableString& tableString, const char* data);
private:
    int width;
    char delimiter;
    stringstream table;
};

