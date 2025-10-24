
#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/* Function to securely clear sensitive data (Rules#1, #7) */
void secure_clear(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

/* Function to create and return a PostgreSQL connection object
 * Parameters are read from environment variables for security (CWE-798 prevention)
 * Returns: PGconn* - Connection object or NULL on failure
 */
PGconn* createPostgresConnection(void) {
    PGconn* conn = NULL;
    char conninfo[1024];
    const char* host = NULL;
    const char* port = NULL;
    const char* dbname = NULL;
    const char* user = NULL;
    const char* password = NULL;
    char* endptr = NULL;
    long port_num = 0;
    int result = 0;
    
    /* Initialize conninfo buffer (Rules#1 - C security checklist) */
    memset(conninfo, 0, sizeof(conninfo));
    
    /* Retrieve connection parameters from environment variables (Rules#3, #4) */
    /* Never hardcode credentials in source code */
    host = getenv("PGHOST");
    port = getenv("PGPORT");
    dbname = getenv("PGDATABASE");
    user = getenv("PGUSER");
    password = getenv("PGPASSWORD");
    
    /* Validate that all required parameters are present (Rules#1 - input validation) */
    if (host == NULL || port == NULL || dbname == NULL || 
        user == NULL || password == NULL) {
        fprintf(stderr, "Error: Missing required environment variables (PGHOST, PGPORT, PGDATABASE, PGUSER, PGPASSWORD)\\n");
        return NULL;
    }
    
    /* Validate parameters are not empty (Rules#1) */
    if (strlen(host) == 0 || strlen(port) == 0 || strlen(dbname) == 0 || 
        strlen(user) == 0 || strlen(password) == 0) {
        fprintf(stderr, "Error: Environment variables cannot be empty\\n");
        return NULL;
    }
    
    /* Validate port is numeric and within valid range (Rules#1) */
    errno = 0;
    port_num = strtol(port, &endptr, 10);
    if (errno != 0 || endptr == port || *endptr != '\\0' || 
        port_num < 1 || port_num > 65535) {
        fprintf(stderr, "Error: Invalid port number\\n");
        return NULL;
    }
    
    /* Build connection string with SSL enforcement (Rules#5, #7) */
    /* Use snprintf to prevent buffer overflow (Rules#1 - C security) */
    result = snprintf(conninfo, sizeof(conninfo),
                     "host=%s port=%s dbname=%s user=%s password=%s sslmode=require connect_timeout=10",
                     host, port, dbname, user, password);
    
    /* Check for truncation (Rules#1) */
    if (result < 0 || (size_t)result >= sizeof(conninfo)) {
        fprintf(stderr, "Error: Connection string too long\\n");
        secure_clear(conninfo, sizeof(conninfo));
        return NULL;
    }
    
    /* Create connection (Rules#14 - error handling) */
    conn = PQconnectdb(conninfo);
    
    /* Clear connection string from memory immediately (Rules#1, #7) */
    secure_clear(conninfo, sizeof(conninfo));
    
    /* Check connection status (Rules#14) */
    if (conn == NULL) {
        fprintf(stderr, "Error: PQconnectdb returned NULL\\n");
        return NULL;
    }
    
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection failed: %s\\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }
    
    return conn;
}

/* Test function with 5 test cases */
int main(void) {
    PGconn* conn = NULL;
    char* original_host = NULL;
    char saved_host[256] = {0};
    char* original_port = NULL;
    char saved_port[256] = {0};
    
    printf("PostgreSQL Connection Test Cases:\\n");
    printf("==================================\\n");
    
    /* Test Case 1: Valid connection (requires env vars to be set) */
    printf("\\nTest 1: Attempting connection with environment variables...\\n");
    conn = createPostgresConnection();
    if (conn != NULL) {
        printf("Test 1 PASSED: Connection established successfully\\n");
        printf("Server version: %d\\n", PQserverVersion(conn));
        PQfinish(conn);
        conn = NULL;
    } else {
        printf("Test 1 FAILED: Could not establish connection\\n");
    }
    
    /* Test Case 2: Missing environment variable */
    printf("\\nTest 2: Testing with missing PGHOST...\\n");
    original_host = getenv("PGHOST");
    if (original_host != NULL) {
        strncpy(saved_host, original_host, sizeof(saved_host) - 1);
        saved_host[sizeof(saved_host) - 1] = '\\0';
    }
    unsetenv("PGHOST");
    conn = createPostgresConnection();
    if (conn == NULL) {
        printf("Test 2 PASSED: Correctly rejected missing parameter\\n");
    } else {
        printf("Test 2 FAILED: Should have rejected connection\\n");
        PQfinish(conn);
        conn = NULL;
    }
    if (saved_host[0] != '\\0') {
        setenv("PGHOST", saved_host, 1);
    }
    
    /* Test Case 3: Invalid port */
    printf("\\nTest 3: Testing with invalid port...\\n");
    original_port = getenv("PGPORT");
    if (original_port != NULL) {
        strncpy(saved_port, original_port, sizeof(saved_port) - 1);
        saved_port[sizeof(saved_port) - 1] = '\\0';
    }
    setenv("PGPORT", "invalid_port", 1);
    conn = createPostgresConnection();
    if (conn == NULL) {
        printf("Test 3 PASSED: Correctly rejected invalid port\\n");
    } else {
        printf("Test 3 FAILED: Should have rejected invalid port\\n");
        PQfinish(conn);
        conn = NULL;
    }
    if (saved_port[0] != '\\0') {
        setenv("PGPORT", saved_port, 1);
    }
    
    /* Test Case 4: Port out of range */
    printf("\\nTest 4: Testing with port out of range...\\n");
    setenv("PGPORT", "99999", 1);
    conn = createPostgresConnection();
    if (conn == NULL) {
        printf("Test 4 PASSED: Correctly rejected out-of-range port\\n");
    } else {
        printf("Test 4 FAILED: Should have rejected out-of-range port\\n");
        PQfinish(conn);
        conn = NULL;
    }
    if (saved_port[0] != '\\0') {
        setenv("PGPORT", saved_port, 1);
    }
    
    /* Test Case 5: Valid connection parameters restored */
    printf("\\nTest 5: Retesting with restored valid parameters...\\n");
    conn = createPostgresConnection();
    if (conn != NULL) {
        printf("Test 5 PASSED: Connection re-established\\n");
        PQfinish(conn);
        conn = NULL;
    } else {
        printf("Test 5 FAILED: Could not re-establish connection\\n");
    }
    
    return 0;
}
