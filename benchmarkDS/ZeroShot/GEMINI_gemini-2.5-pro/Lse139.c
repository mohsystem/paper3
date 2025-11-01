/*
 * Note: This code requires the libpq C library for PostgreSQL.
 * You must have the PostgreSQL development libraries installed.
 * 
 * On Debian/Ubuntu:
 * sudo apt-get install libpq-dev
 *
 * On RedHat/CentOS:
 * sudo yum install postgresql-devel
 *
 * Compilation command:
 * gcc your_file_name.c -o Lse139 -lpq
 *
 * Before running, set the following environment variables:
 * DB_HOST: The database host (e.g., localhost)
 * DB_PORT: The database port (e.g., 5432)
 * DB_NAME: The database name
 * DB_USER: The database user
 * DB_PASSWORD: The database user's password
 *
 * Assumed SQL Schema:
 * CREATE TABLE users (
 *     user_id SERIAL PRIMARY KEY,
 *     username VARCHAR(50) UNIQUE NOT NULL,
 *     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
 * );
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

/**
 * Checks if a user is a moderator by querying the database.
 * Credentials are read from environment variables.
 * Uses PQexecParams (parameterized query) to prevent SQL injection.
 *
 * @param username The username to check.
 * @return 1 if the user is a moderator, 0 otherwise.
 */
int isUserModerator(const char *username) {
    // Ensure username is not NULL or empty
    if (username == NULL || username[0] == '\0') {
        return 0;
    }

    // 1. Get credentials securely from environment variables
    const char *db_host = getenv("DB_HOST");
    const char *db_port = getenv("DB_PORT");
    const char *db_name = getenv("DB_NAME");
    const char *db_user = getenv("DB_USER");
    const char *db_password = getenv("DB_PASSWORD");

    if (!db_host || !db_port || !db_name || !db_user || !db_password) {
        fprintf(stderr, "Error: Database environment variables are not set.\n");
        return 0;
    }

    char conninfo[256];
    snprintf(conninfo, sizeof(conninfo), "host=%s port=%s dbname=%s user=%s password=%s",
             db_host, db_port, db_name, db_user, db_password);

    PGconn *conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return 0;
    }

    int is_moderator = 0;
    const char *sql = "SELECT is_moderator FROM users WHERE username = $1";
    const char *paramValues[1];
    paramValues[0] = username;
    
    // 2. Use a parameterized query to prevent SQL injection
    PGresult *res = PQexecParams(conn,
                                 sql,
                                 1,       // one parameter
                                 NULL,    // let the backend deduce param types
                                 paramValues,
                                 NULL,    // lengths not needed for text format
                                 NULL,    // formats not needed for text format
                                 0);      // result in text format

    // 3. Check query result
    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        if (PQntuples(res) == 1) {
            // In PostgreSQL boolean, 't' means true.
            if (strcmp(PQgetvalue(res, 0, 0), "t") == 0) {
                is_moderator = 1;
            }
        }
    } else {
        // 4. Handle errors properly
        fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
    }
    
    // 5. Clean up resources
    PQclear(res);
    PQfinish(conn);

    return is_moderator;
}

int main() {
    printf("Running test cases...\n");
    printf("Please ensure your database is running and environment variables are set.\n");
    printf("Assumed test data:\n");
    printf(" - user 'admin' is a moderator\n");
    printf(" - user 'bob' is NOT a moderator\n");
    printf(" - user 'charlie' does not exist\n");
    printf("------------------------------------\n");
    
    const char* testUsers[] = {
        "admin",                   // Should be true (1)
        "bob",                     // Should be false (0)
        "charlie",                 // Non-existent user, should be false (0)
        "'; DROP TABLE users; --", // Malicious input, should be false (0) and safe
        NULL                       // NULL input
    };
    int num_tests = sizeof(testUsers) / sizeof(testUsers[0]);

    for (int i = 0; i < num_tests; ++i) {
        int result = isUserModerator(testUsers[i]);
        printf("Is user '%s' a moderator? -> %s\n", 
               testUsers[i] ? testUsers[i] : "(null)",
               result ? "True" : "False");
    }

    return 0;
}