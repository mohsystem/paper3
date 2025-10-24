
#include <iostream>
#include <string>
#include <libpq-fe.h>

PGconn* createPostgresConnection(const std::string& host, int port, const std::string& database, 
                                  const std::string& username, const std::string& password) {
    std::string connInfo = "host=" + host + 
                          " port=" + std::to_string(port) + 
                          " dbname=" + database + 
                          " user=" + username + 
                          " password=" + password;
    
    PGconn* connection = PQconnectdb(connInfo.c_str());
    
    if (PQstatus(connection) != CONNECTION_OK) {
        std::cerr << "Error connecting to database: " << PQerrorMessage(connection) << std::endl;
        PQfinish(connection);
        return nullptr;
    }
    
    std::cout << "Connection established successfully!" << std::endl;
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
