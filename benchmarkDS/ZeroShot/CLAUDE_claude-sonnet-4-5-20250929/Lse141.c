
#include <stdio.h>
#include <string.h>
#include <mysql/mysql.h>

#define SERVER_NAME "localhost"
#define DATABASE_NAME "myDatabase"
#define DATABASE_USER "dbUser"
#define DATABASE_PASSWORD "securePassword123"
#define PORT 3306

MYSQL* getDatabaseConnection() {
    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "MySQL initialization failed\\n");
        return NULL;
    }

    mysql_ssl_set(conn, NULL, NULL, NULL, NULL, NULL);
    
    if (mysql_real_connect(conn, 
                          getServerName(),
                          getDatabaseUser(),
                          getDatabasePassword(),
                          getDatabaseName(),
                          PORT, NULL, CLIENT_SSL) == NULL) {
        fprintf(stderr, "Connection failed: %s\\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }
    
    return conn;
}

const char* getServerName() {
    return SERVER_NAME;
}

const char* getDatabaseName() {
    return DATABASE_NAME;
}

const char* getDatabaseUser() {
    return DATABASE_USER;
}

const char* getDatabasePassword() {
    return DATABASE_PASSWORD;
}

int main() {
    // Test case 1: Get server name
    printf("Test 1 - Server Name: %s\\n", getServerName());

    // Test case 2: Get database name
    printf("Test 2 - Database Name: %s\\n", getDatabaseName());

    // Test case 3: Get database user
    printf("Test 3 - Database User: %s\\n", getDatabaseUser());

    // Test case 4: Get database password (masked for security)
    int len = strlen(getDatabasePassword());
    printf("Test 4 - Database Password: ");
    for (int i = 0; i < len; i++) {
        printf("*");
    }
    printf("\\n");

    // Test case 5: Attempt to get database connection
    MYSQL* conn = getDatabaseConnection();
    if (conn != NULL) {
        printf("Test 5 - Database Connection: Successful\\n");
        mysql_close(conn);
    } else {
        printf("Test 5 - Database Connection: Failed\\n");
    }

    return 0;
}
