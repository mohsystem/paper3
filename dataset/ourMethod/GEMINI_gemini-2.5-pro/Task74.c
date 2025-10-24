#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * To compile and run this code, you need the libpq C library.
 * On Debian/Ubuntu: sudo apt-get install libpq-dev
 * On macOS (Homebrew): brew install libpq
 *
 * Compile command:
 * gcc -o task74 Task74.c -I/usr/include/postgresql -lpq
 * (The -I path may vary depending on your system)
 *
 * Before running, set the following environment variables:
 * export PG_HOST="localhost"
 * export PG_PORT="5432"
 * export PG_DBNAME="testdb"
 * export PG_USER="testuser"
 * export PG_PASSWORD="testpassword"
 */

#include "libpq-fe.h"

/**
 * Creates and returns a connection object for a PostgreSQL database.
 *
 * @param host     The database server host.
 * @param port     The database server port.
 * @param dbname   The name of the database.
 * @param user     The username for authentication.
 * @param password The password for authentication.
 * @return A PGconn pointer, or NULL if connection fails. The caller is responsible for freeing it with PQfinish.
 */
PGconn* getConnection(const char* host, const char* port, const char* dbname, const char* user, const char* password) {
    // Rule #13: Avoid hardcoding credentials. Credentials are passed as parameters.
    if (!host || !port || !dbname || !user || !password) {
        fprintf(stderr, "Error: All connection parameters must be provided.\n");
        return NULL;
    }
    
    // Rule #1 & #2: Ensure buffer sizes are checked before copy operations. Use snprintf for safety.
    // We need space for keys, values, spaces, and null terminator. 256 is plenty.
    char conn_info[256];
    int ret = snprintf(conn_info, sizeof(conn_info),
                     "host=%s port=%s dbname=%s user=%s password=%s sslmode=require",
                     host, port, dbname, user, password);

    if (ret < 0 || (size_t)ret >= sizeof(conn_info)) {
        fprintf(stderr, "Error: Connection string is too long or encoding error occurred.\n");
        return NULL;
    }

    // Connect to the database
    PGconn* conn = PQconnectdb(conn_info);

    // Rule #14: Check the connection status and handle errors gracefully.
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s", PQerrorMessage(conn));
        PQfinish(conn); // Free the PGconn object on failure
        return NULL;
    }

    printf("Connection to %s established successfully.\n", dbname);
    return conn;
}

void run_test(const char* test_name,
              const char* host, const char* port, const char* dbname,
              const char* user, const char* password)
{
    printf("\n[%s]\n", test_name);
    if (!host || strlen(host) == 0) {
        printf("Skipping %s: Environment variables not set.\n", test_name);
        return;
    }
    PGconn* conn = getConnection(host, port, dbname, user, password);
    if (conn != NULL) {
        printf("Connection object is valid.\n");
        PQfinish(conn); // Clean up the connection
    } else {
        printf("Failed to get connection object as expected or an error occurred.\n");
    }
}


int main() {
    printf("--- Running Test Cases ---\n");
    
    // Rule #12: Use environment variables to manage sensitive credentials
    const char* db_host = getenv("PG_HOST");
    const char* db_port = getenv("PG_PORT");
    const char* db_name = getenv("PG_DBNAME");
    const char* db_user = getenv("PG_USER");
    const char* db_password = getenv("PG_PASSWORD");

    // Test Case 1: Successful connection using environment variables
    run_test("Test Case 1: Successful Connection", db_host, db_port, db_name, db_user, db_password);

    // Test Case 2: Connection with a wrong password
    run_test("Test Case 2: Wrong Password", db_host, db_port, db_name, db_user, "wrongpassword");

    // Test Case 3: Connection to a non-existent database
    run_test("Test Case 3: Non-existent Database", db_host, db_port, "nonexistentdb", db_user, db_password);
    
    // Test Case 4: Connection to a wrong host
    run_test("Test Case 4: Wrong Host", "127.0.0.99", db_port, db_name, db_user, db_password);

    // Test Case 5: Connection with a NULL user
    run_test("Test Case 5: NULL User", db_host, db_port, db_name, NULL, db_password);

    return 0;
}