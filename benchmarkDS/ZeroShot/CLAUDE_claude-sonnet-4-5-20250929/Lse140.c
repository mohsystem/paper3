
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

// Function to check if username exists in database
int checkUsernameExists(const char* dbHost, const char* dbUser, 
                       const char* dbPassword, const char* dbName,
                       const char* username) {
    MYSQL* conn = NULL;
    MYSQL_STMT* stmt = NULL;
    MYSQL_BIND bind[1];
    MYSQL_BIND result[1];
    int count = 0;
    int exists = 0;
    
    // Initialize MySQL connection
    conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "MySQL initialization failed\\n");
        return 0;
    }
    
    // Connect to database
    if (!mysql_real_connect(conn, dbHost, dbUser, dbPassword, dbName, 0, NULL, 0)) {
        fprintf(stderr, "Connection failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }
    
    // Prepare parameterized query
    const char* query = "SELECT COUNT(*) FROM users WHERE username = ?";
    stmt = mysql_stmt_init(conn);
    
    if (!stmt || mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Statement preparation failed: %s\\n", mysql_stmt_error(stmt));
        if (stmt) mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    // Bind parameters
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username;
    bind[0].buffer_length = strlen(username);
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "Binding parameters failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    // Execute query
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Query execution failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    // Bind result
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = (char*)&count;
    
    if (mysql_stmt_bind_result(stmt, result)) {
        fprintf(stderr, "Binding result failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    // Fetch result
    if (mysql_stmt_fetch(stmt) == 0) {
        exists = (count > 0) ? 1 : 0;
    }
    
    // Cleanup
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return exists;
}

int main() {
    // Database connection parameters (adjust as needed)
    const char* dbHost = "localhost";
    const char* dbUser = "root";
    const char* dbPassword = "password";
    const char* dbName = "testdb";
    
    // Test cases
    printf("Test Case 1: %d\\n", checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "admin"));
    printf("Test Case 2: %d\\n", checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "user123"));
    printf("Test Case 3: %d\\n", checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "testuser"));
    printf("Test Case 4: %d\\n", checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "john_doe"));
    printf("Test Case 5: %d\\n", checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "nonexistent"));
    
    return 0;
}
