
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

typedef struct {
    char* db_host;
    char* db_user;
    char* db_password;
    char* db_name;
} DBConfig;

int checkIfModerator(const char* userId) {
    MYSQL* conn;
    MYSQL_STMT* stmt;
    MYSQL_BIND bind[1];
    MYSQL_BIND result_bind[1];
    int is_moderator = 0;
    int result = 0;
    
    conn = mysql_init(NULL);
    
    if (!mysql_real_connect(conn, "localhost", "root", "password", 
                           "webapp_db", 0, NULL, 0)) {
        fprintf(stderr, "Connection error: %s\\n", mysql_error(conn));
        return 0;
    }
    
    const char* query = "SELECT is_moderator FROM users WHERE user_id = ?";
    stmt = mysql_stmt_init(conn);
    
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "Prepare error: %s\\n", mysql_stmt_error(stmt));
        mysql_close(conn);
        return 0;
    }
    
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)userId;
    bind[0].buffer_length = strlen(userId);
    
    mysql_stmt_bind_param(stmt, bind);
    mysql_stmt_execute(stmt);
    
    memset(result_bind, 0, sizeof(result_bind));
    result_bind[0].buffer_type = MYSQL_TYPE_LONG;
    result_bind[0].buffer = (char*)&is_moderator;
    
    mysql_stmt_bind_result(stmt, result_bind);
    
    if (mysql_stmt_fetch(stmt) == 0) {
        result = is_moderator;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return result;
}

int main() {
    printf("Test Case 1: %d\\n", checkIfModerator("1"));
    printf("Test Case 2: %d\\n", checkIfModerator("2"));
    printf("Test Case 3: %d\\n", checkIfModerator("3"));
    printf("Test Case 4: %d\\n", checkIfModerator("4"));
    printf("Test Case 5: %d\\n", checkIfModerator("5"));
    
    return 0;
}
