
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

/**
 * Creates and returns a secure PostgreSQL database connection
 * @param host Database host
 * @param port Database port
 * @param database Database name
 * @param username Database username
 * @param password Database password
 * @param useSSL Whether to use SSL connection (1 for true, 0 for false)
 * @return PGconn* PostgreSQL connection pointer, NULL on validation error
 */
PGconn* createPostgresConnection(const char* host, int port,
                                const char* database,
                                const char* username,
                                const char* password,
                                int useSSL) {
    // Validate input parameters
    if (host == NULL || strlen(host) == 0) {
        fprintf(stderr, "Error: Host cannot be NULL or empty\\n");
        return NULL;
    }
    
    if (database == NULL || strlen(database) == 0) {
        fprintf(stderr, "Error: Database name cannot be NULL or empty\\n");
        return NULL;
    }
    
    if (username == NULL || strlen(username) == 0) {
        fprintf(stderr, "Error: Username cannot be NULL or empty\\n");
        return NULL;
    }
    
    if (password == NULL) {
        fprintf(stderr, "Error: Password cannot be NULL\\n");
        return NULL;
    }
    
    if (port < 1 || port > 65535) {
        fprintf(stderr, "Error: Port must be between 1 and 65535\\n");
        return NULL;
    }
    
    // Build connection string with secure parameters
    char conninfo[1024];
    int result = snprintf(conninfo, sizeof(conninfo),
                         "host=%s port=%d dbname=%s user=%s password=%s connect_timeout=10%s",
                         host, port, database, username, password,
                         useSSL ? " sslmode=require" : "");
    
    if (result < 0 || result >= sizeof(conninfo)) {
        fprintf(stderr, "Error: Connection string too long\\n");
        return NULL;
    }
    
    // Create connection
    PGconn* conn = PQconnectdb(conninfo);
    
    // Clear sensitive data from memory
    memset(conninfo, 0, sizeof(conninfo));
    
    // Check connection status
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection failed: %s\\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }
    
    return conn;
}

void testConnection(const char* testName,
                   const char* host, int port,
                   const char* database,
                   const char* username,
                   const char* password,
                   int useSSL) {
    printf("%s\\n", testName);
    
    PGconn* conn = createPostgresConnection(host, port, database,
                                           username, password, useSSL);
    
    if (conn != NULL) {
        printf("✓ Connection created successfully\\n");
        PQfinish(conn);
        printf("✓ Connection closed successfully\\n");
    } else {
        printf("✗ Connection failed\\n");
    }
    
    printf("\\n");
}

int main() {
    printf("=== PostgreSQL Connection Test Cases ===\\n\\n");
    
    // Test Case 1: Standard connection (simulated)
    testConnection("Test 1: Standard Connection",
                  "localhost", 5432, "testdb", "postgres", "password", 0);
    
    // Test Case 2: SSL Connection
    testConnection("Test 2: SSL Connection",
                  "localhost", 5432, "testdb", "postgres", "password", 1);
    
    // Test Case 3: Invalid port
    testConnection("Test 3: Invalid Port",
                  "localhost", -1, "testdb", "postgres", "password", 0);
    
    // Test Case 4: NULL host
    testConnection("Test 4: NULL Host",
                  NULL, 5432, "testdb", "postgres", "password", 0);
    
    // Test Case 5: Empty database
    testConnection("Test 5: Empty Database",
                  "localhost", 5432, "", "postgres", "password", 0);
    
    return 0;
}
