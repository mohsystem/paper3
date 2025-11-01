#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

static const char* getenv_or_default(const char* key, const char* defval) {
    const char* v = getenv(key);
    if (v == NULL || *v == '\0') return defval;
    return v;
}

int is_moderator(const char* username) {
    const char* host = getenv_or_default("DB_HOST", "localhost");
    const char* port = getenv_or_default("DB_PORT", "5432");
    const char* db   = getenv_or_default("DB_NAME", "postgres");
    const char* user = getenv_or_default("DB_USER", "postgres");
    const char* pass = getenv_or_default("DB_PASS", "");

    char conninfo[512];
    snprintf(conninfo, sizeof(conninfo), "host=%s port=%s dbname=%s user=%s password=%s",
             host, port, db, user, pass);

    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        if (conn) PQfinish(conn);
        return 0;
    }

    const char* paramValues[1];
    paramValues[0] = username;

    PGresult* res = PQexecParams(
        conn,
        "SELECT is_moderator FROM users WHERE username = $1 LIMIT 1",
        1,               /* number of params */
        NULL,            /* param types */
        paramValues,     /* param values */
        NULL,            /* param lengths */
        NULL,            /* param formats */
        0                /* text results */
    );

    int result = 0;
    if (PQresultStatus(res) == PGRES_TUPLES_OK && PQntuples(res) > 0) {
        char* val = PQgetvalue(res, 0, 0);
        if (val) {
            if (strcmp(val, "t") == 0 || strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0) {
                result = 1;
            }
        }
    }

    if (res) PQclear(res);
    PQfinish(conn);
    return result;
}

int main(void) {
    const char* users[5] = {"alice", "bob", "charlie", "diana", "eve"};
    for (int i = 0; i < 5; ++i) {
        int v = is_moderator(users[i]);
        printf("User: %s -> %s\n", users[i], v ? "true" : "false");
    }
    return 0;
}