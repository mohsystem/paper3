#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

PGconn* get_postgres_connection(const char* host, int port, const char* database, const char* user, const char* password) {
    char conninfo[1024];
    snprintf(conninfo, sizeof(conninfo),
             "host=%s port=%d dbname=%s user=%s password=%s connect_timeout=3",
             host, port, database, user, password ? password : "");
    PGconn* conn = PQconnectdb(conninfo);
    return conn;
}

typedef struct {
    const char* label;
    const char* host;
    int port;
    const char* db;
    const char* user;
    const char* pass;
} TestCase;

int main(void) {
    TestCase tests[5] = {
        {"Local default port, default db with empty password", "localhost", 5432, "postgres", "postgres", ""},
        {"Local default port with sample credentials", "127.0.0.1", 5432, "postgres", "user", "pass"},
        {"Remote host example", "db.example.com", 5432, "mydb", "myuser", "mypass"},
        {"Wrong port", "localhost", 6543, "postgres", "postgres", "postgres"},
        {"Nonexistent database", "localhost", 5432, "nonexistent", "postgres", "postgres"}
    };

    for (int i = 0; i < 5; ++i) {
        printf("Test: %s\n", tests[i].label);
        PGconn* conn = get_postgres_connection(tests[i].host, tests[i].port, tests[i].db, tests[i].user, tests[i].pass);
        if (PQstatus(conn) == CONNECTION_OK) {
            printf("  Success: Connected.\n");
        } else {
            printf("  Failed: %s", PQerrorMessage(conn));
        }
        PQfinish(conn);
    }

    return 0;
}