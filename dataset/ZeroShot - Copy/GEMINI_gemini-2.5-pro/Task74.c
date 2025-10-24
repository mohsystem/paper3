#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

// Note: This code requires the libpq library (PostgreSQL C client library).
// On Debian/Ubuntu: sudo apt-get install libpq-dev
// On macOS (Homebrew): brew install libpq
// Compile with: gcc your_file.c -o your_app -I/usr/include/postgresql -lpq

/**
 * Creates and returns a connection object for a Postgres database.
 * For security, this function attempts to connect using SSL.
 *
 * @param host The database server host.
 * @param port The database server port.
 * @param dbname The name of the database.
 * @param user The username for authentication.
 * @param password The password for authentication.
 * @return A PGconn pointer, or NULL if the connection fails.
 *         The caller is responsible for freeing the connection with PQfinish().
 */
PGconn* getPostgresConnection(const char* host, const char* port, const char* dbname, const char* user, const char* password) {
    // Use a sufficiently large buffer for the connection string
    char conninfo[256];

    // Securely format the connection string. snprintf prevents buffer overflows.
    // The 'sslmode=require' option enforces an SSL connection for security.
    int ret = snprintf(conninfo, sizeof(conninfo),
                       "host=%s port=%s dbname=%s user=%s password=%s sslmode=require",
                       host ? host : "",
                       port ? port : "",
                       dbname ? dbname : "",
                       user ? user : "",
                       password ? password : "");

    if (ret >= (int)sizeof(conninfo)) {
        fprintf(stderr, "Connection string truncated. Buffer is too small.\n");
        return NULL;
    }

    // Make a connection to the database
    PGconn* conn = PQconnectdb(conninfo);

    // Check to see that the backend connection was successfully made
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        // Free the PGconn object on failure
        PQfinish(conn);
        return NULL;
    }

    printf("Connection to %s established successfully.\n", dbname);
    return conn;
}

int main() {
    // --- Test Cases ---
    // These are placeholder credentials and will likely fail to connect.
    // Replace with your actual database details to test a successful connection.
    PGconn* conn;

    // Test Case 1: Valid-looking but likely incorrect credentials
    printf("--- Test Case 1 ---\n");
    conn = getPostgresConnection("localhost", "5432", "mydatabase", "myuser", "mypassword");
    if (conn != NULL) {
        printf("Test Case 1: Connection object returned.\n");
        PQfinish(conn); // Clean up the connection
    } else {
        printf("Test Case 1: Failed to get connection object.\n");
    }
    printf("\n");

    // Test Case 2: Invalid host
    printf("--- Test Case 2 ---\n");
    conn = getPostgresConnection("invalid-host", "5432", "testdb", "user2", "pass2");
    if (conn != NULL) {
        printf("Test Case 2: Connection object returned.\n");
        PQfinish(conn);
    } else {
        printf("Test Case 2: Failed to get connection object.\n");
    }
    printf("\n");

    // Test Case 3: Invalid port
    printf("--- Test Case 3 ---\n");
    conn = getPostgresConnection("localhost", "9999", "proddb", "user3", "pass3");
    if (conn != NULL) {
        printf("Test Case 3: Connection object returned.\n");
        PQfinish(conn);
    } else {
        printf("Test Case 3: Failed to get connection object.\n");
    }
    printf("\n");

    // Test Case 4: NULL password
    printf("--- Test Case 4 ---\n");
    conn = getPostgresConnection("localhost", "5432", "analytics", "user4", NULL);
    if (conn != NULL) {
        printf("Test Case 4: Connection object returned.\n");
        PQfinish(conn);
    } else {
        printf("Test Case 4: Failed to get connection object.\n");
    }
    printf("\n");

    // Test Case 5: Empty parameters
    printf("--- Test Case 5 ---\n");
    conn = getPostgresConnection("", "", "", "", "");
    if (conn != NULL) {
        printf("Test Case 5: Connection object returned.\n");
        PQfinish(conn);
    } else {
        printf("Test Case 5: Failed to get connection object.\n");
    }
    printf("\n");

    return 0;
}