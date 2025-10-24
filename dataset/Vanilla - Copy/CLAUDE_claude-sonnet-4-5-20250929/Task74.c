
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>

PGconn* createPostgresConnection(const char* host, int port, const char* database, 
                                  const char* username, const char* password) {
    char connInfo[512];
    snprintf(connInfo, sizeof(connInfo), 
             "host=%s port=%d dbname=%s user=%s password=%s",
             host, port, database, username, password);
    
    PGconn* connection = PQconnectdb(connInfo);
    
    if (PQstatus(connection) != CONNECTION_OK) {
        fprintf(stderr, "Error connecting to database: %s\\n", PQerrorMessage(connection));
        PQfinish(connection);
        return NULL;
    }
    
    printf("Connection established successfully!\\n");
    return connection;
}

int main() {
    // Test case 1: Standard connection
    PGconn* conn1 = createPostgresConnection("localhost", 5432, "testdb", "postgres", "password");
    if (conn1) PQfinish(conn1);

    // Test case 2: Custom port
    PGconn* conn2 = createPostgresConnection("localhost", 5433, "mydb", "admin", "admin123");
    if (conn2) PQfinish(conn2);

    // Test case 3: Remote host
    PGconn* conn3 = createPostgresConnection("192.168.1.100", 5432, "production", "dbuser", "securepass");
    if (conn3) PQfinish(conn3);

    // Test case 4: Different database
    PGconn* conn4 = createPostgresConnection("localhost", 5432, "analytics", "analyst", "analyze123");
    if (conn4) PQfinish(conn4);

    // Test case 5: Development environment
    PGconn* conn5 = createPostgresConnection("dev.example.com", 5432, "devdb", "developer", "dev@2024");
    if (conn5) PQfinish(conn5);

    return 0;
}
