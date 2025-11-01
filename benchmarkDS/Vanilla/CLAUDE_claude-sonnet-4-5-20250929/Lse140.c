
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

int checkUsernameExists(const char* username, const char* host, const char* user, 
                        const char* password, const char* database) {
    MYSQL* conn = NULL;
    MYSQL_STMT* stmt = NULL;
    MYSQL_BIND bind[1];
    MYSQL_BIND result[1];
    int count = 0;
    int exists = 0;
    
    // Initialize MySQL connection
    conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\\n");
        return 0;
    }
    
    // Connect to the database
    if (mysql_real_connect(conn, host, user, password, database, 0, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }
    
    // Prepare the query
    const char* query = "SELECT COUNT(*) FROM users WHERE username = ?";
    stmt = mysql_stmt_init(conn);
    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    // Bind parameters
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username;
    bind[0].buffer_length = strlen(username);
    
    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    // Execute the query
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    // Bind result
    memset(result, 0, sizeof(result));
    result[0].buffer_type = MYSQL_TYPE_LONG;
    result[0].buffer = &count;
    
    if (mysql_stmt_bind_result(stmt, result)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed: %s\\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return 0;
    }
    
    // Fetch result
    if (mysql_stmt_fetch(stmt) == 0) {
        exists = (count > 0) ? 1 : 0;
    }
    
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return exists;
}

int main() {
    const char* host = "localhost";
    const char* user = "root";
    const char* password = "password";
    const char* database = "testdb";
    
    // Test cases
    printf("Test 1: %d\\n", checkUsernameExists("john_doe", host, user, password, database));
    printf("Test 2: %d\\n", checkUsernameExists("jane_smith", host, user, password, database));
    printf("Test 3: %d\\n", checkUsernameExists("admin", host, user, password, database));
    printf("Test 4: %d\\n", checkUsernameExists("test_user", host, user, password, database));
    printf("Test 5: %d\\n", checkUsernameExists("nonexistent", host, user, password, database));
    
    return 0;
}
