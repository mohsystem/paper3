
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mysql/mysql.h>  // For MySQL, adjust for other databases

bool isUserModerator(const char* username) {
    const char* dbHost = getenv("DB_HOST");
    const char* dbUser = getenv("DB_USER");
    const char* dbPassword = getenv("DB_PASSWORD");
    const char* dbName = getenv("DB_NAME");
    
    if (!dbHost || !dbUser || !dbPassword || !dbName) {
        fprintf(stderr, "Database credentials not found in environment variables\\n");
        return false;
    }
    
    MYSQL* conn = mysql_init(NULL);
    
    if (!conn) {
        fprintf(stderr, "MySQL initialization failed\\n");
        return false;
    }
    
    if (!mysql_real_connect(conn, dbHost, dbUser, dbPassword, dbName, 0, NULL, 0)) {
        fprintf(stderr, "Database connection failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    const char* query = "SELECT is_moderator FROM users WHERE username = ?";
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Statement preparation failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    unsigned long length = strlen(username);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username;
    bind[0].buffer_length = length;
    bind[0].length = &length;
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "Parameter binding failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "Query execution failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    bool isModerator = false;
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    
    result[0].buffer_type = MYSQL_TYPE_TINY;
    result[0].buffer = &isModerator;
    
    mysql_stmt_bind_result(stmt, result);
    
    bool returnValue = false;
    if (mysql_stmt_fetch(stmt) == 0) {
        returnValue = isModerator;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    return returnValue;
}

int main() {
    // Test cases
    printf("Test 1: %d\\n", isUserModerator("admin_user"));
    printf("Test 2: %d\\n", isUserModerator("moderator1"));
    printf("Test 3: %d\\n", isUserModerator("regular_user"));
    printf("Test 4: %d\\n", isUserModerator("test_mod"));
    printf("Test 5: %d\\n", isUserModerator("guest_user"));
    
    return 0;
}
