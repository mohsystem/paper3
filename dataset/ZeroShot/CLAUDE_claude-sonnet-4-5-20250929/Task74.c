
#include <stdio.h>
#include <string.h>
#include <libpq-fe.h>

PGconn* createPostgresConnection(const char* host, int port, 
                                 const char* database,
                                 const char* username, 
                                 const char* password) {
    char connInfo[512];
    snprintf(connInfo, sizeof(connInfo), 
             "host=%s port=%d dbname=%s user=%s password=%s sslmode=require connect_timeout=10",
             host, port, database, username, password);
    
    PGconn* conn = PQconnectdb(connInfo);
    
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Connection failed: %s\\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }
    
    printf("Connected to PostgreSQL database successfully!\\n");
    return conn;
}

int main() {
    // Test case 1: Local connection
    PGconn* conn1 = createPostgresConnection("localhost", 5432, "testdb", "user1", "pass1");
    if (conn1) PQfinish(conn1);
    
    // Test case 2: Custom port
    PGconn* conn2 = createPostgresConnection("localhost", 5433, "mydb", "admin", "admin123");
    if (conn2) PQfinish(conn2);
    
    // Test case 3: Remote connection
    PGconn* conn3 = createPostgresConnection("192.168.1.100", 5432, "proddb", "dbuser", "securepass");
    if (conn3) PQfinish(conn3);
    
    // Test case 4: Different database
    PGconn* conn4 = createPostgresConnection("localhost", 5432, "analytics", "analyst", "analyze123");
    if (conn4) PQfinish(conn4);
    
    // Test case 5: Another local instance
    PGconn* conn5 = createPostgresConnection("127.0.0.1", 5432, "devdb", "developer", "dev@2024");
    if (conn5) PQfinish(conn5);
    
    return 0;
}
