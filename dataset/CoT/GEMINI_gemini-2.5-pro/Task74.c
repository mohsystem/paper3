#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

/*
 * To compile and run this code, you need PostgreSQL's C client library (libpq).
 * On Debian/Ubuntu: sudo apt-get install libpq-dev
 * On RedHat/CentOS: sudo dnf install postgresql-devel
 *
 * Example compilation:
 * gcc -o task74 Task74.c -I/usr/include/postgresql -lpq
 */

/**
 * Establishes a connection to a PostgreSQL database.
 *
 * @param conninfo The connection string (e.g., "dbname=testdb user=postgres password=secret ...").
 * @return A PGconn pointer to the connection object, or NULL if the connection fails.
 *         The caller is responsible for freeing the PGconn object using PQfinish().
 *
 * Security Note: In a real application, the connection string should be built from
 * securely sourced credentials and not hardcoded. Always use parameterized queries
 * (e.g., PQexecParams) with the returned connection to prevent SQL injection.
 */
PGconn* connectToPostgres(const char* conninfo) {
    PGconn *conn = PQconnectdb(conninfo);

    // Check to see that the backend connection was successfully made
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection to database failed: %s\n", PQerrorMessage(conn));
        PQfinish(conn); // Free the connection object even on failure
        return NULL;
    }

    return conn;
}

int main() {
    // --- IMPORTANT ---
    // Replace these placeholder values with your actual PostgreSQL database details.
    // For the test cases to work as expected, you need a running PostgreSQL server.
    const char* HOST = "localhost";
    const char* PORT = "5432";
    const char* DBNAME = "your_db";
    const char* USER = "your_user";
    const char* PASSWORD = "your_password";
    
    // Buffer to hold connection strings
    char conn_str_buffer[256];

    // Test Case 1: Successful Connection
    printf("--- Test Case 1: Attempting a successful connection... ---\n");
    snprintf(conn_str_buffer, sizeof(conn_str_buffer), "host=%s port=%s dbname=%s user=%s password=%s", HOST, PORT, DBNAME, USER, PASSWORD);
    PGconn* conn1 = connectToPostgres(conn_str_buffer);
    if (conn1 != NULL) {
        printf("Test Case 1: Connection successful!\n");
        PQfinish(conn1);
        printf("Test Case 1: Connection closed.\n");
    } else {
        printf("Test Case 1: Connection failed. Please check your credentials and database status.\n");
    }
    printf("\n");

    // Test Case 2: Incorrect Password
    printf("--- Test Case 2: Attempting connection with a wrong password... ---\n");
    snprintf(conn_str_buffer, sizeof(conn_str_buffer), "host=%s port=%s dbname=%s user=%s password=wrong_password", HOST, PORT, DBNAME, USER);
    PGconn* conn2 = connectToPostgres(conn_str_buffer);
    if (conn2 == NULL) {
        printf("Test Case 2: Connection failed as expected.\n");
    } else {
        printf("Test Case 2: Connection unexpectedly succeeded.\n");
        PQfinish(conn2);
    }
    printf("\n");

    // Test Case 3: Incorrect Database Name
    printf("--- Test Case 3: Attempting connection to a non-existent database... ---\n");
    snprintf(conn_str_buffer, sizeof(conn_str_buffer), "host=%s port=%s dbname=non_existent_db user=%s password=%s", HOST, PORT, USER, PASSWORD);
    PGconn* conn3 = connectToPostgres(conn_str_buffer);
    if (conn3 == NULL) {
        printf("Test Case 3: Connection failed as expected.\n");
    } else {
        printf("Test Case 3: Connection unexpectedly succeeded.\n");
        PQfinish(conn3);
    }
    printf("\n");

    // Test Case 4: Incorrect User
    printf("--- Test Case 4: Attempting connection with a wrong user... ---\n");
    snprintf(conn_str_buffer, sizeof(conn_str_buffer), "host=%s port=%s dbname=%s user=wrong_user password=%s", HOST, PORT, DBNAME, PASSWORD);
    PGconn* conn4 = connectToPostgres(conn_str_buffer);
    if (conn4 == NULL) {
        printf("Test Case 4: Connection failed as expected.\n");
    } else {
        printf("Test Case 4: Connection unexpectedly succeeded.\n");
        PQfinish(conn4);
    }
    printf("\n");
    
    // Test Case 5: Incorrect Host
    printf("--- Test Case 5: Attempting connection to a non-existent host... ---\n");
    snprintf(conn_str_buffer, sizeof(conn_str_buffer), "host=nonexistenthost port=%s dbname=%s user=%s password=%s", PORT, DBNAME, USER, PASSWORD);
    PGconn* conn5 = connectToPostgres(conn_str_buffer);
    if (conn5 == NULL) {
        printf("Test Case 5: Connection failed as expected.\n");
    } else {
        printf("Test Case 5: Connection unexpectedly succeeded.\n");
        PQfinish(conn5);
    }
    printf("\n");

    return 0;
}