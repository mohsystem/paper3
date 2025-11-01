#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

/*
 Returns 1 if moderator, 0 otherwise. On error, returns 0 after printing to stderr.
*/
int is_moderator(const char* username) {
    if (username == NULL || *username == '\0') {
        fprintf(stderr, "username cannot be empty\n");
        return 0;
    }

    const char* host = getenv("DB_HOST");
    const char* port = getenv("DB_PORT");
    const char* db   = getenv("DB_NAME");
    const char* user = getenv("DB_USER");
    const char* pass = getenv("DB_PASSWORD");
    const char* sslm = getenv("DB_SSLMODE");

    if (!host || !*host || !db || !*db || !user || !*user || !pass) {
        fprintf(stderr, "Missing required DB environment variables\n");
        return 0;
    }

    const char* default_port = "5432";
    const char* default_ssl  = "prefer";

    const char* keywords[] = {"host","port","dbname","user","password","sslmode",NULL};
    const char* values[]   = {host, (port && *port) ? port : default_port, db, user, pass, (sslm && *sslm) ? sslm : default_ssl, NULL};

    PGconn* conn = PQconnectdbParams(keywords, values, 0);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "DB connection error\n");
        PQfinish(conn);
        return 0;
    }

    PGresult* setres = PQexec(conn, "SET statement_timeout = '5s'");
    if (PQresultStatus(setres) != PGRES_COMMAND_OK) {
        // ignore
    }
    PQclear(setres);

    const char* sql = "SELECT is_moderator FROM users WHERE username = $1 LIMIT 1";
    const char* params[1] = { username };
    int lengths[1] = { 0 };
    int formats[1] = { 0 };

    PGresult* res = PQexecParams(conn, sql, 1, NULL, params, lengths, formats, 0);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "Query failed\n");
        PQclear(res);
        PQfinish(conn);
        return 0;
    }

    int moderator = 0;
    if (PQntuples(res) > 0) {
        char* v = PQgetvalue(res, 0, 0);
        if (v) {
            for (char* p = v; *p; ++p) {
                if (*p >= 'A' && *p <= 'Z') *p = (char)(*p - 'A' + 'a');
            }
            if (strcmp(v, "t") == 0 || strcmp(v, "true") == 0 || strcmp(v, "1") == 0 || strcmp(v, "yes") == 0) {
                moderator = 1;
            }
        }
    }

    PQclear(res);
    PQfinish(conn);
    return moderator;
}

int main(void) {
    const char* tests[5] = {"alice","bob","charlie","diana","eve"};
    for (int i = 0; i < 5; ++i) {
        int res = is_moderator(tests[i]);
        printf("User %s moderator? %s\n", tests[i], res ? "true" : "false");
    }
    return 0;
}