#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// This code requires libpq, the C library for PostgreSQL.
// On Debian/Ubuntu: sudo apt-get install libpq-dev
// On macOS (Homebrew): brew install libpq
// Compile with: gcc -o program your_file.c -I/usr/include/postgresql -lpq
#include "libpq-fe.h"

// Define a boolean type for C
typedef enum { false, true } bool;

/**
 * Connects to a PostgreSQL database using credentials from environment variables
 * and checks if a user is a moderator.
 *
 * Required Environment Variables:
 * DB_HOST: The database host (e.g., "localhost")
 * DB_PORT: The database port (e.g., "5432")
 * DB_NAME: The name of the database (e.g., "appdb")
 * DB_USER: The database username
 * DB_PASS: The database user's password
 *
 * Assumed table schema:
 * CREATE TABLE users (
 *     user_id SERIAL PRIMARY KEY,
 *     username VARCHAR(255) UNIQUE NOT NULL,
 *     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
 * );
 *
 * @param userId The ID of the user to check.
 * @return true if the user is a moderator, false otherwise or if an error occurs.
 */
bool isUserModerator(int userId) {
    const char* host = getenv("DB_HOST");
    const char* port = getenv("DB_PORT");
    const char* dbname = getenv("DB_NAME");
    const char* user = getenv("DB_USER");
    const char* password = getenv("DB_PASS");

    if (!host || !port || !dbname || !user || !password) {
        fprintf(stderr, "Error: Database environment variables are not fully set.\n");
        return false;
    }

    char conninfo[256];
    snprintf(conninfo, sizeof(conninfo), "host=%s port=%s dbname=%s user=%s password=%s",
             host, port, dbname, user, password);

    PGconn *conn = PQconnectdb(conninfo);
    bool isModerator = false;

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        PQfinish(conn);
        return false;
    }

    const char *sql = "SELECT is_moderator FROM users WHERE user_id = $1;";
    char userIdStr[12]; // Buffer for integer to string conversion
    snprintf(userIdStr, sizeof(userIdStr), "%d", userId);

    const char *paramValues[1] = { userIdStr };
    // Use PQexecParams for parameterized queries to prevent SQL injection
    PGresult *res = PQexecParams(conn, sql, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        // Check if we got exactly one row
        if (PQntuples(res) == 1) {
            // Get the value from the first row, first column
            char *value = PQgetvalue(res, 0, 0);
            // PostgreSQL booleans are 't' for true and 'f' for false
            if (value[0] == 't') {
                isModerator = true;
            }
        }
    } else {
        fprintf(stderr, "Query failed: %s", PQerrorMessage(conn));
    }
    
    // Clean up resources
    PQclear(res);
    PQfinish(conn);
    
    return isModerator;
}

void Lse139_main() {
    printf("--- C Test Cases ---\n");
    printf("Note: This requires a running PostgreSQL database, 'libpq' library, and correctly set environment variables.\n");
    printf("See comments in the isUserModerator function for setup details.\n");

    // Test cases (these will only work if you have a corresponding database setup)
    // Assume user 1 is a moderator, user 2 is not, and user 99 does not exist.
    int moderatorUserId = 1;
    int regularUserId = 2;
    int nonExistentUserId = 99;
    int anotherModeratorId = 3;
    int anotherRegularId = 4;

    printf("Is user %d a moderator? %s\n", moderatorUserId, isUserModerator(moderatorUserId) ? "true" : "false");
    printf("Is user %d a moderator? %s\n", regularUserId, isUserModerator(regularUserId) ? "true" : "false");
    printf("Is user %d a moderator? %s\n", nonExistentUserId, isUserModerator(nonExistentUserId) ? "true" : "false");
    printf("Is user %d a moderator? %s\n", anotherModeratorId, isUserModerator(anotherModeratorId) ? "true" : "false");
    printf("Is user %d a moderator? %s\n", anotherRegularId, isUserModerator(anotherRegularId) ? "true" : "false");
}

int main() {
    Lse139_main();
    return 0;
}