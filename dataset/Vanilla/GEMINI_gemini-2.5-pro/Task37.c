/*
NOTE: This code requires the MySQL C API client library (libmysqlclient).
You must install the library and link against it during compilation.
For example, on Linux:
gcc -o task37 your_code.c $(mysql_config --cflags --libs)

DATABASE SETUP:
Before running, you must have a MySQL server running and execute the following SQL:

CREATE DATABASE IF NOT EXISTS testdb;
USE testdb;
CREATE TABLE IF NOT EXISTS users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    email VARCHAR(100) NOT NULL,
    full_name VARCHAR(100)
);
INSERT INTO users (username, email, full_name) VALUES
('alice', 'alice@example.com', 'Alice Smith'),
('bob', 'bob@example.com', 'Bob Johnson'),
('charlie', 'charlie@example.com', 'Charlie Brown'),
('diana', 'diana@example.com', 'Diana Prince')
ON DUPLICATE KEY UPDATE email=VALUES(email), full_name=VALUES(full_name);

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>

// Database connection details - CHANGE THESE
#define DB_HOST "127.0.0.1"
#define DB_USER "user"
#define DB_PASS "password"
#define DB_NAME "testdb"
#define DB_PORT 3306

void getUserInfo(const char *username) {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "mysql_init() failed\n");
        return;
    }

    if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0) == NULL) {
        fprintf(stderr, "mysql_real_connect() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    const char *query = "SELECT username, email, full_name FROM users WHERE username = ?";
    MYSQL_STMT *stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() failed\n");
        mysql_close(conn);
        return;
    }

    if (mysql_stmt_prepare(stmt, query, strlen(query))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    MYSQL_BIND param_bind[1];
    memset(param_bind, 0, sizeof(param_bind));
    
    unsigned long username_len = strlen(username);
    param_bind[0].buffer_type = MYSQL_TYPE_STRING;
    param_bind[0].buffer = (char *)username;
    param_bind[0].buffer_length = username_len;
    param_bind[0].is_null = 0;
    param_bind[0].length = &username_len;

    if (mysql_stmt_bind_param(stmt, param_bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }
    
    char res_username[51], res_email[101], res_full_name[101];
    unsigned long len_username, len_email, len_full_name;
    my_bool is_null_username, is_null_email, is_null_full_name;

    MYSQL_BIND result_bind[3];
    memset(result_bind, 0, sizeof(result_bind));

    result_bind[0].buffer_type = MYSQL_TYPE_STRING;
    result_bind[0].buffer = res_username;
    result_bind[0].buffer_length = sizeof(res_username);
    result_bind[0].length = &len_username;
    result_bind[0].is_null = &is_null_username;

    result_bind[1].buffer_type = MYSQL_TYPE_STRING;
    result_bind[1].buffer = res_email;
    result_bind[1].buffer_length = sizeof(res_email);
    result_bind[1].length = &len_email;
    result_bind[1].is_null = &is_null_email;

    result_bind[2].buffer_type = MYSQL_TYPE_STRING;
    result_bind[2].buffer = res_full_name;
    result_bind[2].buffer_length = sizeof(res_full_name);
    result_bind[2].length = &len_full_name;
    result_bind[2].is_null = &is_null_full_name;

    if (mysql_stmt_bind_result(stmt, result_bind)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    if (mysql_stmt_store_result(stmt)) {
        fprintf(stderr, "mysql_stmt_store_result() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return;
    }

    if (mysql_stmt_fetch(stmt) == 0) { // Success
        printf("User Found:\n");
        printf("  Username: %s\n", is_null_username ? "NULL" : res_username);
        printf("  Email: %s\n", is_null_email ? "NULL" : res_email);
        printf("  Full Name: %s\n", is_null_full_name ? "NULL" : res_full_name);
    } else {
        printf("User '%s' not found.\n", username);
    }

    mysql_stmt_close(stmt);
    mysql_close(conn);
}


int main(void) {
    const char *testUsernames[] = {"alice", "bob", "eve", "charlie", "diana"};
    
    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Searching for user: %s\n", testUsernames[i]);
        getUserInfo(testUsernames[i]);
    }
    
    return 0;
}