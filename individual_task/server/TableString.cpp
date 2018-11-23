#include "TableString.h"

TableString::TableString(int width, char delimiter) {
    this->width = width;
    this->delimiter = delimiter;
}

TableString::~TableString() = default;

string TableString::getString() {
    return table.str();
}

TableString &operator<<(TableString &tableString, const char *data) {
    if (strcmp(data, "\n") == 0) {
        tableString.table << data;
    } else {
        tableString.table << left << setw(tableString.width) << data << tableString.delimiter;
    }
    return tableString;
}