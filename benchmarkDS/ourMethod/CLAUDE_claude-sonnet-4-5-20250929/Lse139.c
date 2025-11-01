
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <stdbool.h>

/**
 * Check if a user is a moderator by querying the database.
 * Uses environment variables for database credentials.
 * 
 * @param username The username to check (must be non-NULL and non-empty)
 * @return true if user is a moderator, false otherwise
 */
bool isUserModerator(const char* username) {
    // Validate input pointer (Rules#1, Rules#8)
    if (username == NULL) {
        fprintf(stderr, "Username cannot be NULL\\n");
        return false;
    }
    
    // Validate input is not empty (Rules#8)
    size_t usernameLen = strlen(username);
    if (usernameLen == 0) {
        fprintf(stderr, "Username cannot be empty\\n");
        return false;
    }
    
    // Limit username length to prevent potential attacks (Rules#8)
    if (usernameLen > 255) {
        fprintf(stderr, "Username exceeds maximum length\\n");
        return false;
    }
    
    // Retrieve database credentials from environment variables (Rules#4, Rules#5)
    const char* dbHost = getenv("DB_HOST");
    const char* dbName = getenv("DB_NAME");
    const char* dbUser = getenv("DB_USER");
    const char* dbPassword = getenv("DB_PASSWORD");
    const char* dbPortStr = getenv("DB_PORT");
    
    // Validate that credentials are available (Rules#8)
    if (dbHost == NULL || dbName == NULL || dbUser == NULL || dbPassword == NULL) {
        fprintf(stderr, "Database credentials not found in environment variables\\n");
        return false;
    }
    
    unsigned int dbPort = dbPortStr ? (unsigned int)atoi(dbPortStr) : 3306;
    
    // Initialize MySQL connection (Rules#1)
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "MySQL initialization failed\\n");
        return false;
    }
    
    // Establish database connection (Rules#11)
    if (mysql_real_connect(conn, dbHost, dbUser, dbPassword, dbName, dbPort, NULL, 0) == NULL) {
        fprintf(stderr, "Database connection failed\\n");
        mysql_close(conn);
        return false;
    }
    
    // Prepare statement to prevent SQL injection (Rules#1)
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (stmt == NULL) {
        fprintf(stderr, "Statement initialization failed\\n");
        mysql_close(conn);
        return false;
    }
    
    // Use parameterized query (Rules#1)
    const char* query = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query)) != 0) {
        fprintf(stderr, "Statement preparation failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    // Bind parameter (Rules#1, Rules#8)
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    unsigned long paramLen = usernameLen;
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username;
    bind[0].buffer_length = usernameLen;
    bind[0].length = &paramLen;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        fprintf(stderr, "Parameter binding failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    // Execute query
    if (mysql_stmt_execute(stmt) != 0) {
        fprintf(stderr, "Query execution failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    // Bind result
    MYSQL_BIND resultBind[1];
    memset(resultBind, 0, sizeof(resultBind));
    
    bool isModerator = false;
    my_bool isNull = 0;
    
    resultBind[0].buffer_type = MYSQL_TYPE_TINY;
    resultBind[0].buffer = &isModerator;
    resultBind[0].is_null = &isNull;
    
    if (mysql_stmt_bind_result(stmt, resultBind) != 0) {
        fprintf(stderr, "Result binding failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    // Fetch result
    int fetchResult = mysql_stmt_fetch(stmt);
    bool result = false;
    
    if (fetchResult == 0 && !isNull) {
        result = isModerator;
    }
    
    // Clean up resources (Rules#11)
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return result;
}

int main(void) {
    // For testing purposes only - set environment variables
    // In production, these should be actual environment variables
    #ifdef _WIN32
    _putenv("DB_HOST=localhost");
    _putenv("DB_NAME=testdb");
    _putenv("DB_USER=testuser");
    _putenv("DB_PASSWORD=testpass");
    _putenv("DB_PORT=3306");
    #else
    setenv("DB_HOST", "localhost", 1);
    setenv("DB_NAME", "testdb", 1);
    setenv("DB_USER", "testuser", 1);
    setenv("DB_PASSWORD", "testpass", 1);
    setenv("DB_PORT", "3306", 1);
    #endif
    
    // Test case 1: Valid moderator user
    printf("Test 1 - Valid moderator: %s\\n", isUserModerator("admin_user") ? "true" : "false");
    
    // Test case 2: Valid non-moderator user
    printf("Test 2 - Regular user: %s\\n", isUserModerator("regular_user") ? "true" : "false");
    
    // Test case 3: Non-existent user
    printf("Test 3 - Non-existent user: %s\\n", isUserModerator("nonexistent") ? "true" : "false");
    
    // Test case 4: Empty username
    printf("Test 4 - Empty username: %s\\n", isUserModerator("") ? "true" : "false");
    
    // Test case 5: NULL username
    printf("Test 5 - NULL username: %s\\n", isUserModerator(NULL) ? "true" : "false");
    
    return 0;
}
