#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <postgresql/libpq-fe.h> // PostgreSQL C library

/*
To compile and run this code, you need:
1.  A PostgreSQL database.
2.  A 'users' table in the database with schema:
    CREATE TABLE users (
        id SERIAL PRIMARY KEY,
        username VARCHAR(50) UNIQUE NOT NULL,
        is_moderator BOOLEAN NOT NULL DEFAULT FALSE
    );
3.  The PostgreSQL C development library installed.
    On Debian/Ubuntu: sudo apt-get install libpq-dev
    On RedHat/CentOS: sudo yum install postgresql-devel
4.  The following environment variables set:
    DB_HOST=your_database_host
    DB_PORT=your_database_port (e.g., 5432)
    DB_NAME=your_database_name
    DB_USER=your_username
    DB_PASSWORD=your_password
5.  Compile with the command:
    gcc your_file_name.c -o lse139_c -lpq
*/

bool isUserModerator(const char* username) {
    bool isModerator = false;

    const char* host = getenv("DB_HOST");
    const char* port = getenv("DB_PORT");
    const char* dbName = getenv("DB_NAME");
    const char* user = getenv("DB_USER");
    const char* password = getenv("DB_PASSWORD");

    if (!host || !port || !dbName || !user || !password) {
        fprintf(stderr, "Database environment variables are not set.\n");
        return false;
    }

    // Allocate enough space for the connection string
    char conninfo[256];
    snprintf(conninfo, sizeof(conninfo), "host=%s port=%s dbname=%s user=%s password=%s",
             host, port, dbName, user, password);

    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return false;
    }

    const char *paramValues[1];
    paramValues[0] = username;
    
    const char *query = "SELECT is_moderator FROM users WHERE username = $1";

    PGresult *res = PQexecParams(conn, query, 1, NULL, paramValues, NULL, NULL, 0);

    if (PQresultStatus(res) == PGRES_TUPLES_OK) {
        if (PQntuples(res) == 1) {
            // The result is 't' for true and 'f' for false
            if (strcmp(PQgetvalue(res, 0, 0), "t") == 0) {
                isModerator = true;
            }
        }
    } else {
        fprintf(stderr, "Query failed: %s\n", PQerrorMessage(conn));
    }

    PQclear(res);
    PQfinish(conn);

    return isModerator;
}

int main() {
    // --- Test Cases ---
    // Note: These tests require a database connection and pre-populated data.
    // For example, insert a user 'admin' who is a moderator and 'guest' who is not.
    // INSERT INTO users (username, is_moderator) VALUES ('admin', TRUE);
    // INSERT INTO users (username, is_moderator) VALUES ('guest', FALSE);
    // INSERT INTO users (username, is_moderator) VALUES ('moderator1', TRUE);

    printf("Checking 5 test cases. This requires a live database connection and pre-configured environment variables.\n");
    
    // Test Case 1: A user who is a moderator
    const char* user1 = "admin";
    printf("Is '%s' a moderator? %s\n", user1, isUserModerator(user1) ? "true" : "false");

    // Test Case 2: A user who is not a moderator
    const char* user2 = "guest";
    printf("Is '%s' a moderator? %s\n", user2, isUserModerator(user2) ? "true" : "false");

    // Test Case 3: Another user who is a moderator
    const char* user3 = "moderator1";
    printf("Is '%s' a moderator? %s\n", user3, isUserModerator(user3) ? "true" : "false");

    // Test Case 4: A user who does not exist in the database
    const char* user4 = "nonexistent_user";
    printf("Is '%s' a moderator? %s\n", user4, isUserModerator(user4) ? "true" : "false");

    // Test Case 5: A user with a potentially malicious name (handled by PQexecParams)
    const char* user5 = "'; DROP TABLE users; --";
    printf("Is '%s' a moderator? %s\n", user5, isUserModerator(user5) ? "true" : "false");

    return 0;
}