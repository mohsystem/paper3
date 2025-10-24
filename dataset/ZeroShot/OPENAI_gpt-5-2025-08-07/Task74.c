#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libpq-fe.h>
#ifdef _WIN32
#include <windows.h>
#endif

static int is_valid_hostname(const char* host) {
    if (!host) return 0;
    size_t len = strlen(host);
    if (len == 0 || len > 253) return 0;
    if (host[0] == '[' && host[len - 1] == ']') {
        if (len <= 2) return 0;
        for (size_t i = 1; i < len - 1; ++i) {
            char c = host[i];
            if (!isxdigit((unsigned char)c) && c != ':' && c != '.') return 0;
        }
        return 1;
    }
    for (size_t i = 0; i < len; ++i) {
        char c = host[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '-' || c == '_')) return 0;
    }
    return 1;
}

static int is_valid_dbname(const char* db) {
    if (!db) return 0;
    size_t len = strlen(db);
    if (len == 0 || len > 63) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = db[i];
        if (!(isalnum((unsigned char)c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static const char* normalize_sslmode(const char* mode) {
    if (!mode || !*mode) return "require";
    // to lower copy
    static char buf[32];
    size_t n = strlen(mode);
    if (n >= sizeof(buf)) n = sizeof(buf) - 1;
    for (size_t i = 0; i < n; ++i) buf[i] = (char)tolower((unsigned char)mode[i]);
    buf[n] = '\0';
    if (!strcmp(buf, "disable") || !strcmp(buf, "allow") || !strcmp(buf, "prefer")
        || !strcmp(buf, "require") || !strcmp(buf, "verify-ca") || !strcmp(buf, "verify-full")) {
        return buf;
    }
    return "require";
}

PGconn* get_postgres_connection(
    const char* host,
    int port,
    const char* dbname,
    const char* user,
    const char* password,
    const char* sslmode,
    const char* sslrootcert,
    int connect_timeout_seconds
) {
    if (!is_valid_hostname(host)) return NULL;
    if (port < 1 || port > 65535) return NULL;
    if (!is_valid_dbname(dbname)) return NULL;
    if (!user || !*user) return NULL;
    if (connect_timeout_seconds < 1 || connect_timeout_seconds > 600) connect_timeout_seconds = 10;

    char portStr[8];
    snprintf(portStr, sizeof(portStr), "%d", port);

    char timeoutStr[8];
    snprintf(timeoutStr, sizeof(timeoutStr), "%d", connect_timeout_seconds);

    const char* mode = normalize_sslmode(sslmode);

    const char* keywords[] = {
        "host", "port", "dbname", "user", "password",
        "sslmode", "sslrootcert", "connect_timeout", "application_name", NULL
    };
    const char* values[] = {
        host,
        portStr,
        dbname,
        user,
        password ? password : "",
        mode,
        (sslrootcert && *sslrootcert) ? sslrootcert : NULL,
        timeoutStr,
        "Task74",
        NULL
    };

    PGconn* conn = PQconnectdbParams(keywords, values, 0);
    if (!conn) return NULL;
    if (PQstatus(conn) != CONNECTION_OK) {
        PQfinish(conn);
        return NULL;
    }
    return conn;
}

static const char* getenv_or(const char* key, const char* defv) {
    const char* v = getenv(key);
    return v ? v : defv;
}

static int getenv_or_int(const char* key, int defv) {
    const char* v = getenv(key);
    if (!v) return defv;
    char* endptr = NULL;
    long val = strtol(v, &endptr, 10);
    if (!endptr || *endptr != '\0') return defv;
    if (val < 0 || val > 65535) return defv;
    return (int)val;
}

int main(void) {
    struct Test {
        const char* host;
        int port;
        const char* db;
        const char* user;
        const char* pass;
        const char* sslmode;
        const char* sslrootcert;
        int timeout;
    } tests[5];

    tests[0] = (struct Test){"localhost", 5432, "postgres", "postgres", "", "require", NULL, 5};
    tests[1] = (struct Test){"invalid.localdomain", 5432, "postgres", "postgres", "secret", "require", NULL, 5};
    tests[2] = (struct Test){"localhost", 1, "postgres", "postgres", "secret", "require", NULL, 3};
    tests[3] = (struct Test){"localhost", 5432, "invalid_db", "postgres", "secret", "require", NULL, 5};
    tests[4] = (struct Test){
        getenv_or("PGHOST", "localhost"),
        getenv_or_int("PGPORT", 5432),
        getenv_or("PGDATABASE", "postgres"),
        getenv_or("PGUSER", "postgres"),
        getenv_or("PGPASSWORD", ""),
        getenv_or("PGSSLMODE", "require"),
        getenv_or("PGSSLROOTCERT", NULL),
        getenv_or_int("PGCONNECT_TIMEOUT", 5)
    };

    for (int i = 0; i < 5; ++i) {
        PGconn* conn = get_postgres_connection(
            tests[i].host, tests[i].port, tests[i].db, tests[i].user,
            tests[i].pass, tests[i].sslmode, tests[i].sslrootcert, tests[i].timeout
        );
        if (conn) {
            printf("Test %d: Connection successful\n", i + 1);
            PQfinish(conn);
        } else {
            printf("Test %d: Connection failed or invalid input\n", i + 1);
        }
    }
    return 0;
}