/* Chain-of-Through:
 * 1) Problem: Provide a function that creates and returns a PostgreSQL connection (PGconn*).
 * 2) Security: No hardcoded secrets; do not print credentials; enforce SSL by default; timeouts; escape conninfo values.
 * 3) Secure coding: Build conninfo safely; set sslmode, connect_timeout, application_name.
 * 4) Review: Ensure PQfinish used; validate inputs; avoid leaking secrets.
 * 5) Output: Final function; main with 5 test cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

static char* escape_conninfo_value(const char* v) {
    if (v == NULL) {
        char* empty = (char*)malloc(3);
        if (!empty) return NULL;
        strcpy(empty, "''");
        return empty;
    }
    size_t len = strlen(v);
    // Worst case every char is escaped, plus quotes
    size_t cap = len * 2 + 3;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    out[j++] = '\'';
    for (size_t i = 0; i < len; ++i) {
        if (v[i] == '\'' || v[i] == '\\') {
            out[j++] = '\\';
        }
        out[j++] = v[i];
    }
    out[j++] = '\'';
    out[j] = '\0';
    return out;
}

PGconn* get_postgres_connection(
    const char* host,
    int port,
    const char* database,
    const char* user,
    const char* password,
    int require_ssl,       /* 1=require, 0=prefer */
    int connect_timeout    /* seconds */
) {
    if (!host || !*host) {
        fprintf(stderr, "host must not be empty\n");
        return NULL;
    }
    if (!database || !*database) {
        fprintf(stderr, "database must not be empty\n");
        return NULL;
    }
    if (!user || !*user) {
        fprintf(stderr, "user must not be empty\n");
        return NULL;
    }
    if (port <= 0 || port > 65535) {
        fprintf(stderr, "port must be in range 1..65535\n");
        return NULL;
    }
    if (connect_timeout < 1) connect_timeout = 1;

    char* ehost = escape_conninfo_value(host);
    char* edb = escape_conninfo_value(database);
    char* euser = escape_conninfo_value(user);
    char* epass = password && *password ? escape_conninfo_value(password) : NULL;
    if (!ehost || !edb || !euser || (password && *password && !epass)) {
        free(ehost); free(edb); free(euser); free(epass);
        return NULL;
    }

    const char* sslmode = require_ssl ? "require" : "prefer";

    // Build conninfo string
    char buf[1024];
    int n = snprintf(buf, sizeof(buf),
                     "host=%s port=%d dbname=%s user=%s%s%s sslmode=%s connect_timeout=%d application_name='Task74App'",
                     ehost, port, edb, euser,
                     epass ? " password=" : "",
                     epass ? epass : "",
                     sslmode, connect_timeout);
    free(ehost); free(edb); free(euser); free(epass);
    if (n < 0 || (size_t)n >= sizeof(buf)) {
        fprintf(stderr, "conninfo too long\n");
        return NULL;
    }

    PGconn* conn = PQconnectdb(buf);
    return conn; /* Caller must check status and PQfinish */
}

static void run_test(const char* label,
                     const char* host, int port,
                     const char* db, const char* user,
                     const char* password, int require_ssl, int timeout) {
    printf("Test: %s\n", label);
    PGconn* conn = get_postgres_connection(host, port, db, user, password, require_ssl, timeout);
    if (!conn) {
        printf("  Failed to create connection object\n");
        return;
    }
    if (PQstatus(conn) == CONNECTION_OK) {
        printf("  Connection established successfully\n");
    } else {
        printf("  Failed to connect: %s", PQerrorMessage(conn));
    }
    PQfinish(conn);
    printf("  Connection closed\n");
}

int main(void) {
    const char* envUser = getenv("PGUSER");
    const char* envPass = getenv("PGPASSWORD");
    const char* user = envUser ? envUser : "postgres";
    const char* pass = envPass ? envPass : "";

    // 5 test cases (these may fail depending on the environment)
    run_test("Local default (SSL required, short timeout)", "127.0.0.1", 5432, "postgres", user, pass, 1, 2);
    run_test("Invalid port", "127.0.0.1", 1, "postgres", user, pass, 1, 2);
    run_test("Invalid host", "no-such-host.invalid", 5432, "postgres", user, pass, 1, 2);
    run_test("Non-SSL prefer", "localhost", 5432, "postgres", user, pass, 0, 2);
    run_test("Custom DB name", "localhost", 5432, "mydb", user, pass, 1, 2);

    return 0;
}