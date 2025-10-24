
#include <iostream>
#include <string>
#include <stdexcept>
#include <libpq-fe.h>
#include <cstring>

class Task74 {
public:
    /**
     * Creates and returns a secure PostgreSQL database connection
     * @param host Database host
     * @param port Database port
     * @param database Database name
     * @param username Database username
     * @param password Database password
     * @param useSSL Whether to use SSL connection
     * @return PGconn* PostgreSQL connection pointer
     * @throws std::invalid_argument if parameters are invalid
     * @throws std::runtime_error if connection fails
     */
    static PGconn* createPostgresConnection(const std::string& host, int port,
                                           const std::string& database,
                                           const std::string& username,
                                           const std::string& password,
                                           bool useSSL) {
        // Validate input parameters
        if (host.empty()) {
            throw std::invalid_argument("Host cannot be empty");
        }
        
        if (database.empty()) {
            throw std::invalid_argument("Database name cannot be empty");
        }
        
        if (username.empty()) {
            throw std::invalid_argument("Username cannot be empty");
        }
        
        if (port < 1 || port > 65535) {
            throw std::invalid_argument("Port must be between 1 and 65535");
        }
        
        // Build connection string with secure parameters
        std::string conninfo = "host=" + host +
                              " port=" + std::to_string(port) +
                              " dbname=" + database +
                              " user=" + username +
                              " password=" + password +
                              " connect_timeout=10";
        
        // Add SSL mode if required
        if (useSSL) {
            conninfo += " sslmode=require";
        }
        
        // Create connection
        PGconn* conn = PQconnectdb(conninfo.c_str());
        
        // Check connection status
        if (PQstatus(conn) != CONNECTION_OK) {
            std::string error = PQerrorMessage(conn);
            PQfinish(conn);
            throw std::runtime_error("Connection failed: " + error);
        }
        
        return conn;
    }
    
    static void testConnection(const std::string& testName,
                             const std::string& host, int port,
                             const std::string& database,
                             const std::string& username,
                             const std::string& password,
                             bool useSSL) {
        std::cout << testName << std::endl;
        try {
            PGconn* conn = createPostgresConnection(host, port, database,
                                                   username, password, useSSL);
            std::cout << "✓ Connection created successfully" << std::endl;
            
            if (conn != nullptr) {
                PQfinish(conn);
                std::cout << "✓ Connection closed successfully" << std::endl;
            }
        } catch (const std::invalid_argument& e) {
            std::cout << "✗ Validation Error: " << e.what() << std::endl;
        } catch (const std::runtime_error& e) {
            std::cout << "✗ Runtime Error: " << e.what() << std::endl;
        }
        std::cout << std::endl;
    }
};

int main() {
    std::cout << "=== PostgreSQL Connection Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Standard connection (simulated)
    Task74::testConnection("Test 1: Standard Connection",
                          "localhost", 5432, "testdb", "postgres", "password", false);
    
    // Test Case 2: SSL Connection
    Task74::testConnection("Test 2: SSL Connection",
                          "localhost", 5432, "testdb", "postgres", "password", true);
    
    // Test Case 3: Invalid port
    Task74::testConnection("Test 3: Invalid Port",
                          "localhost", -1, "testdb", "postgres", "password", false);
    
    // Test Case 4: Empty host
    Task74::testConnection("Test 4: Empty Host",
                          "", 5432, "testdb", "postgres", "password", false);
    
    // Test Case 5: Empty database
    Task74::testConnection("Test 5: Empty Database",
                          "localhost", 5432, "", "postgres", "password", false);
    
    return 0;
}
