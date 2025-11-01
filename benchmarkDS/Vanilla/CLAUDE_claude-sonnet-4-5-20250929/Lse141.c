
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#define SERVER_NAME "localhost"
#define DATABASE_NAME "myDatabase"
#define DATABASE_USER "root"
#define DATABASE_PASSWORD "password123"

// Function that returns a database connection
MYSQL* get_database_connection() {
    MYSQL* conn = mysql_init(NULL);
    
    if (conn == NULL) {
        fprintf(stderr, "MySQL initialization failed\\n");
        return NULL;
    }
    
    if (mysql_real_connect(conn, get_server_name(), 
                          get_database_user(),
                          get_database_password(),
                          get_database_name(), 
                          0, NULL, 0) == NULL) {
        fprintf(stderr, "Database connection failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }
    
    return conn;
}

// Function that returns the server name
const char* get_server_name() {
    return SERVER_NAME;
}

// Function that returns the database name
const char* get_database_name() {
    return DATABASE_NAME;
}

// Function that returns the database user
const char* get_database_user() {
    return DATABASE_USER;
}

// Function that returns the database password
const char* get_database_password() {
    return DATABASE_PASSWORD;
}

int main() {
    // Test case 1: Get server name
    printf("Test 1 - Server Name: %s\\n", get_server_name());
    
    // Test case 2: Get database name
    printf("Test 2 - Database Name: %s\\n", get_database_name());
    
    // Test case 3: Get database user
    printf("Test 3 - Database User: %s\\n", get_database_user());
    
    // Test case 4: Get database password
    printf("Test 4 - Database Password: %s\\n", get_database_password());
    
    // Test case 5: Get database connection
    MYSQL* conn = get_database_connection();
    if (conn != NULL) {
        printf("Test 5 - Database Connection: Successful\\n");
        mysql_close(conn);
    } else {
        printf("Test 5 - Database Connection: Failed\\n");
    }
    
    return 0;
}
