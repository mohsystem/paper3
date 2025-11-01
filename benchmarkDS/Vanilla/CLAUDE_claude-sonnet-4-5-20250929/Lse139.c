
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <stdbool.h>

bool isModerator(const char* username) {
    const char* dbUrl = getenv("DB_URL");
    const char* dbUser = getenv("DB_USER");
    const char* dbPassword = getenv("DB_PASSWORD");
    const char* dbName = getenv("DB_NAME");
    
    if (!dbUrl || !dbUser || !dbPassword) {
        fprintf(stderr, "Database credentials not found in environment variables\\n");
        return false;
    }
    
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "mysql_init() failed\\n");
        return false;
    }
    
    if (!mysql_real_connect(conn, dbUrl, dbUser, dbPassword, dbName, 0, NULL, 0)) {
        fprintf(stderr, "Connection failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return false;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    const char* query = "SELECT is_moderator FROM users WHERE username = ?";
    
    if (!stmt || mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Statement preparation failed: %s\\n", mysql_error(conn));
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
    
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    
    bool isMod = false;
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    
    result[0].buffer_type = MYSQL_TYPE_TINY;
    result[0].buffer = &isMod;
    
    mysql_stmt_bind_result(stmt, result);
    
    bool returnValue = false;
    if (mysql_stmt_fetch(stmt) == 0) {
        returnValue = isMod;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    return returnValue;
}

int main() {
    // Test cases
    printf("Test 1: %d\\n", isModerator("admin"));
    printf("Test 2: %d\\n", isModerator("moderator1"));
    printf("Test 3: %d\\n", isModerator("user123"));
    printf("Test 4: %d\\n", isModerator("guest"));
    printf("Test 5: %d\\n", isModerator("nonexistent"));
    
    return 0;
}
