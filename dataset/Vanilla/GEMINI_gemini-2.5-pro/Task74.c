#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

/**
 * Creates and returns a connection handle for a Postgres database.
 * Note: You need to have libpq (PostgreSQL C library) installed and link against it.
 * Example compile command: gcc your_file.c -o your_app -I/usr/include/postgresql -lpq
 *
 * @param host     The database server host.
 * @param port     The database server port as a string.
 * @param dbName   The name of the database.
 * @param user     The username for authentication.
 * @param password The password for authentication.
 * @return A PGconn connection handle, or NULL if connection fails. The caller is responsible for freeing it with PQfinish().
 */
PGconn* create_connection(const char* host, const char* port, const char* dbName, const char* user, const char* password) {
    char conninfo[256];
    snprintf(conninfo, sizeof(conninfo), "host=%s port=%s dbname=%s user=%s password=%s",
             host, port, dbName, user, password);

    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        // In a real application, you would log this error.
        // fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    return conn;
}

void run_test_case(int case_num, const char* host, const char* port, const char* dbname, const char* user, const char* password) {
    printf("--- Test Case %d ---\n", case_num);
    printf("Attempting to connect to postgresql://%s@%s:%s/%s\n", user, host, port, dbname);
    
    PGconn* conn = create_connection(host, port, dbname, user, password);

    if (conn != NULL) {
        printf("Connection successful!\n");
        PQfinish(conn);
        printf("Connection closed.\n");
    } else {
        printf("Connection failed.\n");
    }
    printf("\n");
}

int main() {
    // --- Test Cases ---

    // Test Case 1: Standard localhost connection (likely to fail without a running DB and correct credentials)
    run_test_case(1, "localhost", "5432", "testdb", "user", "password");

    // Test Case 2: Using IP address for localhost
    run_test_case(2, "127.0.0.1", "5432", "postgres", "postgres", "admin123");

    // Test Case 3: Different database and user
    run_test_case(3, "localhost", "5432", "mydatabase", "dbuser", "secret");

    // Test Case 4: Invalid host to simulate connection failure
    run_test_case(4, "nonexistent.host.com", "5432", "testdb", "user", "password");

    // Test Case 5: Invalid port
    run_test_case(5, "localhost", "9999", "testdb", "user", "password");

    return 0;
}