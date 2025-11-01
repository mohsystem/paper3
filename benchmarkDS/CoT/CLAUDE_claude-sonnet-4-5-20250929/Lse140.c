
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include <stdbool.h>

#define DB_HOST "localhost"
#define DB_USER "root"
#define DB_PASSWORD "password"
#define DB_NAME "testdb"

bool checkUsernameExists(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return false;
    }
    
    MYSQL* conn = mysql_init(NULL);
    if (!conn) {
        fprintf(stderr, "MySQL initialization failed\\n");
        return false;
    }
    
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, 
                            DB_NAME, 0, NULL, 0)) {
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
    
    const char* query = "SELECT COUNT(*) FROM users WHERE username = ?";
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
        fprintf(stderr, "Statement execution failed\\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    int count = 0;
    MYSQL_BIND result[1];
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = (char*)&count;
    
    mysql_stmt_bind_result(stmt, result);
    mysql_stmt_fetch(stmt);
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return count > 0;
}

int main() {
    const char* testUsers[] = {"admin", "john_doe", "'; DROP TABLE users; --", 
                               "user123", "nonexistent"};
    
    printf("Testing username existence:\\n");
    for (int i = 0; i < 5; i++) {
        bool exists = checkUsernameExists(testUsers[i]);
        printf("Username '%s' exists: %s\\n", testUsers[i], 
               exists ? "true" : "false");
    }
    
    return 0;
}
