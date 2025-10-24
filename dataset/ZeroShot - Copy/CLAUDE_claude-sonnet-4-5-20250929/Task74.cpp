
#include <iostream>
#include <string>
#include <libpq-fe.h>

PGconn* createPostgresConnection(const std::string& host, int port, 
                                 const std::string& database,
                                 const std::string& username, 
                                 const std::string& password) {
    std::string connInfo = "host=" + host + 
                          " port=" + std::to_string(port) +
                          " dbname=" + database +
                          " user=" + username +
                          " password=" + password +
                          " sslmode=require" +
                          " connect_timeout=10";
    
    PGconn* conn = PQconnectdb(connInfo.c_str());
    
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return nullptr;
    }
    
    std::cout << "Connected to PostgreSQL database successfully!" << std::endl;
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
