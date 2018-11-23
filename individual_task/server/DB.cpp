#include "DB.h"

DB::DB() {
    int libpq_ver = PQlibVersion();
    BOOST_LOG_TRIVIAL(debug) << "Version of libpq: " << libpq_ver;

    conn = PQconnectdb("user=postgres password=postgres host=localhost dbname=payments");
    if (PQstatus(conn) != CONNECTION_OK) {
        BOOST_LOG_TRIVIAL(error) << "ERORR on database " << PQerrorMessage(conn);
        delete this;
        exit(1);
    }
}

DB::~DB() {
    if (res != NULL)
        PQclear(res);

    if (conn != NULL)
        PQfinish(conn);
}

const char *DB::findUser(char *login, char *pass, string &name, bool &isClient) {
    const char *query =
            "SELECT * FROM account "
            "where login = $1 and pass = $2;";
    const char *params[2] = {login, pass};
    res = PQexecParams(conn, query, 2, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        BOOST_LOG_TRIVIAL(error) << "ERROR on db find user request";
        PQclear(res);
        return "-1";
    }
    if (PQntuples(res) != 1) {
        BOOST_LOG_TRIVIAL(error) << "User not found";
        return "-1";
    }
    BOOST_LOG_TRIVIAL(info) << "User found";

    int nameNum = PQfnumber(res, "name");
    name = PQgetvalue(res, 0, nameNum);

    int isClientNum = PQfnumber(res, "is_client");
    isClient = strcmp(PQgetvalue(res, 0, isClientNum), "t") == 0;

    int idNum = PQfnumber(res, "id");
    char *id = PQgetvalue(res, 0, idNum);
    return id;
}

int DB::saveData(const char *clientId, string month, string year, const char *metersData) {
    const char *query =
            "INSERT INTO bill (id, month, year, meters_data, client_id) "
            "VALUES (DEFAULT, $1, $2, $3, $4);";
    const char *params[4] = {month.c_str(), year.c_str(), metersData, clientId};
    res = PQexecParams(conn, query, 4, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        BOOST_LOG_TRIVIAL(error) << "ERROR on db when save data";
        PQclear(res);
        return -1;
    }
    return 0;
}

int DB::getData(const char *clientId, const char *monthToShow, string *result){
    if(strcmp(monthToShow, "0") == 0){
        const char *query =
                "SELECT meters_data, month, year FROM bill "
                "WHERE client_id = $1 "
                "ORDER BY year, month DESC;";
        const char *params[1] = {clientId};
        res = PQexecParams(conn, query, 1, NULL, params, NULL, NULL, 0);
    } else {
        const char *query =
                "SELECT meters_data, month, year FROM bill "
                "WHERE client_id = $1 "
                "ORDER BY year, month DESC "
                "LIMIT $2;";
        const char *params[2] = {clientId, monthToShow};
        res = PQexecParams(conn, query, 2, NULL, params, NULL, NULL, 0);
    }

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        BOOST_LOG_TRIVIAL(error) << "ERROR on db when read data";
        PQclear(res);
        return -1;
    }
    if (PQntuples(res) == 0) {
        *result = "Данных не обнаружено";
        return 0;
    }

    TableString table;
    table << "\n" << "meters_data" << "delta" << "month" << "year" << "\n";
    int nrows = PQntuples(res);
    for(int i = 0; i < nrows; i++)
    {
        const char* meters_data = PQgetvalue(res, i, 0);
        const char* month = PQgetvalue(res, i, 1);
        const char* year = PQgetvalue(res, i, 2);
        table << meters_data;
        if(i != nrows - 1){
            const char* next_data = PQgetvalue(res, i + 1, 0);
            int delta = atoi(meters_data) - atoi(next_data);
            table << to_string(delta).c_str();
        } else {
            table << "";
        }
        table << month << year << "\n";
    }

    *result = table.getString();
    return 0;
}

const char * DB::getClientItByName(const char *login) {
    const char *query =
            "SELECT id FROM account "
            "where login = $1;";
    const char *params[1] = {login};
    res = PQexecParams(conn, query, 1, NULL, params, NULL, NULL, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        BOOST_LOG_TRIVIAL(error) << "ERROR on db find user request";
        PQclear(res);
        return "-1";
    }
    if (PQntuples(res) != 1) {
        BOOST_LOG_TRIVIAL(error) << "User not found";
        return "-1";
    }
    BOOST_LOG_TRIVIAL(info) << "User found";

    int idNum = PQfnumber(res, "id");
    char *id = PQgetvalue(res, 0, idNum);
    return id;
}

int DB::getDefaulters(const char *month, const char *year, string *result) {
    const char *query =
            "SELECT name, login FROM account "
            "INNER JOIN ( "
                "SELECT client_id FROM bill temp "
                "WHERE $1 NOT IN (SELECT month FROM bill WHERE temp.client_id = client_id) "
                "OR $2 NOT IN (SELECT year FROM bill WHERE temp.client_id = client_id) "
                "GROUP BY client_id) not_present "
            "ON id = not_present.client_id;";
    const char *params[2] = {month, year};
    res = PQexecParams(conn, query, 2, NULL, params, NULL, NULL, 0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        BOOST_LOG_TRIVIAL(error) << "ERROR on db when read data";
        PQclear(res);
        return -1;
    }
    if (PQntuples(res) == 0) {
        *result = "Данных не обнаружено";
        return 0;
    }

    TableString table;
    table << "\n" << "account" << "name" << "\n";
    int nrows = PQntuples(res);
    for(int i = 0; i < nrows; i++)
    {
        const char* name = PQgetvalue(res, i, 0);
        const char* account = PQgetvalue(res, i, 1);
        table << account << name << "\n";
    }

    *result = table.getString();
    return 0;
}
