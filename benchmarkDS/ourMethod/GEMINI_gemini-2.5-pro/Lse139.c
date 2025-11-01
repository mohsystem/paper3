#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <libpq-fe.h>

// To compile this code, you need the libpq library (PostgreSQL C client library).
// On Debian/Ubuntu: sudo apt-get install libpq-dev
// Compile command: gcc -o moderator_check_c moderator_check_c.c -lpq
//
// Before running, set the following environment variables:
// export DB_HOST=localhost
// export DB_PORT=5432
// export DB_NAME=yourdbname
// export DB_USER=youruser
// export DB_PASS=yourpassword
//
// And create the required table and data in your PostgreSQL database:
// CREATE TABLE users (
//     id SERIAL PRIMARY KEY,
//     username VARCHAR(50) UNIQUE NOT NULL,
//     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
// );
// INSERT INTO users (username, is_moderator) VALUES ('alice', TRUE);
// INSERT INTO users (username, is_moderator) VALUES ('bob', FALSE);

/**
 * Checks if a user is a moderator by querying a database.
 * Credentials and connection details are read from environment variables.
 * Uses PQexecParams to prevent SQL injection.
 *
 * @param username The username to check.
 * @return true if the user exists and is a moderator, false otherwise.
 */
bool isUserModerator(const char* username) {
    // Rule #1: Ensure all input is validated.
    if (username == NULL || username[0] == '\0') {
        fprintf(stderr, "Error: Username cannot be null or empty.\n");
        return false;
    }

    // Rules #6, #7: Use environment variables for credentials.
    const char* db_host = getenv("DB_HOST");
    const char* db_port = getenv("DB_PORT");
    const char* db_name = getenv("DB_NAME");
    const char* db_user = getenv("DB_USER");
    const char* db_pass = getenv("DB_PASS");

    if (!db_host || !db_port || !db_name || !db_user || !db_pass) {
        fprintf(stderr, "Error: Database environment variables not set.\n");
        fprintf(stderr, "Please set DB_HOST, DB_PORT, DB_NAME, DB_USER, and DB_PASS.\n");
        return false;
    }

    char conninfo[256];
    int ret = snprintf(conninfo, sizeof(conninfo), "host=%s port=%s dbname=%s user=%s password=%s",
             db_host, db_port, db_name, db_user, db_pass);
    if (ret >= sizeof(conninfo)) {
        fprintf(stderr, "Error: Connection string is too long.\n");
        return false;
    }

    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return false;
    }

    const char *sql = "SELECT is_moderator FROM users WHERE username = $1";
    const char *paramValues[1] = {username};

    // Use PQexecParams to prevent SQL injection.
    PGresult *res = PQexecParams(conn,
                                 sql,
                                 1,       // one parameter
                                 NULL,    // let the backend deduce param types
                                 paramValues,
                                 NULL,    // lengths not needed for text format
                                 NULL,    // formats not needed for text format
                                 0);      // result in text format

    bool is_moderator = false;

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        fprintf(stderr, "SELECT failed: %s\n", PQerrorMessage(conn));
        // Fall through to cleanup
    } else {
        // Check if exactly one row is returned.
        if (PQntuples(res) == 1) {
            // Result is 't' for true, 'f' for false.
            if (strcmp(PQgetvalue(res, 0, 0), "t") == 0) {
                is_moderator = true;
            }
        }
    }
    
    // Cleanup resources.
    PQclear(res);
    PQfinish(conn);

    return is_moderator;
}

int main() {
    printf("Running moderator check tests...\n");
    printf("---------------------------------\n");
    printf("This test assumes a PostgreSQL database is running and\n");
    printf("the required environment variables and table are set up.\n");
    printf("---------------------------------\n");

    // 5 Test cases
    const char* test_users[] = {
        "alice",          // Expected: true (existing moderator)
        "bob",            // Expected: false (existing non-moderator)
        "eve",            // Expected: false (non-existent user)
        "' OR '1'='1",    // Expected: false (SQL injection attempt)
        ""                // Expected: false (invalid input)
    };
    int num_tests = sizeof(test_users) / sizeof(test_users[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* user = test_users[i];
        printf("Checking if user '%s' is a moderator... %s\n",
               strlen(user) == 0 ? "<empty>" : user,
               isUserModerator(user) ? "true" : "false");
    }

    return 0;
}