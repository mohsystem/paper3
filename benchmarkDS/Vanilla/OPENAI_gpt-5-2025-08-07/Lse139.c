#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

static const char* getenv_or_empty(const char* key) {
    const char* v = getenv(key);
    return v ? v : "";
}

bool is_moderator(const char* username) {
    if (username == NULL || *username == '\0') return false;
    const char* host = getenv_or_empty("DB_HOST");
    const char* port = getenv("DB_PORT");
    const char* db   = getenv_or_empty("DB_NAME");
    const char* user = getenv_or_empty("DB_USER");
    const char* pwd  = getenv_or_empty("DB_PASSWORD");
    if (port == NULL || *port == '\0') port = "5432";
    if (*host == '\0' || *db == '\0' || *user == '\0') return false;

    char conninfo[1024];
    snprintf(conninfo, sizeof(conninfo), "host=%s port=%s dbname=%s user=%s password=%s", host, port, db, user, pwd);

    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        return false;
    }

    const char* sql = "SELECT is_moderator FROM users WHERE username = $1 LIMIT 1";
    const char* paramValues[1] = { username };
    PGresult* res = PQexecParams(conn, sql, 1, NULL, paramValues, NULL, NULL, 0);

    bool result = false;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) >= 1) {
        char* val = PQgetvalue(res, 0, 0);
        if (val != NULL) {
            if (strcmp(val, "t") == 0 || strcmp(val, "true") == 0 || strcmp(val, "1") == 0) {
                result = true;
            }
        }
    }

    PQclear(res);
    PQfinish(conn);
    return result;
}

int main(void) {
    const char* testUsers[5] = {"alice", "bob", "charlie", "dana", "eric"};
    for (int i = 0; i < 5; ++i) {
        bool r = is_moderator(testUsers[i]);
        printf("%s -> %s\n", testUsers[i], r ? "true" : "false");
    }
    return 0;
}