
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <stdbool.h>

bool isUserModerator(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return false;
    }
    
    const char* dbHost = getenv("DB_HOST");
    const char* dbUser = getenv("DB_USER");
    const char* dbPassword = getenv("DB_PASSWORD");
    const char* dbName = getenv("DB_NAME");
    
    if (!dbHost || !dbUser || !dbPassword || !dbName) {
        fprintf(stderr, "Database credentials not configured\\n");
        return false;
    }
    
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "MySQL initialization failed\\n");
        return false;
    }
    
    if (!mysql_real_connect(conn, dbHost, dbUser, dbPassword, dbName, 0, NULL, 0)) {
        fprintf(stderr, "Database connection failed\\n");
        mysql_close(conn);
        return false;
    }
    
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "Statement initialization failed\\n");
        mysql_close(conn);
        return false;
    }
    
    const char* query = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Statement preparation failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    
    unsigned long length = strlen(username);
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username;
    bind[0].buffer_length = strlen(username);
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
    result[0].buffer = (char*)&isModerator;
    result[0].is_null = 0;
    result[0].length = 0;
    
    mysql_stmt_bind_result(stmt, result);
    
    int fetchResult = mysql_stmt_fetch(stmt);
    bool returnValue = (fetchResult == 0) ? isModerator : false;
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return returnValue;
}

int main() {
    printf("Test 1 - Valid moderator: %d\\n", isUserModerator("admin"));
    printf("Test 2 - Valid non-moderator: %d\\n", isUserModerator("user123"));
    printf("Test 3 - Non-existent user: %d\\n", isUserModerator("nonexistent"));
    printf("Test 4 - NULL username: %d\\n", isUserModerator(NULL));
    printf("Test 5 - Empty username: %d\\n", isUserModerator(""));
    
    return 0;
}
