
#include <libpq-fe.h>
#include <iostream>
#include <cstring>
#include <cstdlib>

// Function to create and return a PostgreSQL connection object
// Parameters are read from environment variables for security (CWE-798 prevention)
// Returns: PGconn* - Connection object or nullptr on failure
PGconn* createPostgresConnection() {
    // Retrieve connection parameters from environment variables (Rules#3, #4)
    // Never hardcode credentials in source code
    const char* host = std::getenv("PGHOST");
    const char* port = std::getenv("PGPORT");
    const char* dbname = std::getenv("PGDATABASE");
    const char* user = std::getenv("PGUSER");
    const char* password = std::getenv("PGPASSWORD");
    
    // Validate that all required parameters are present (Rules#1 - input validation)
    if (!host || !port || !dbname || !user || !password) {
        std::cerr << "Error: Missing required environment variables (PGHOST, PGPORT, PGDATABASE, PGUSER, PGPASSWORD)" << std::endl;
        return nullptr;
    }
    
    // Validate port is numeric and within valid range (Rules#1)
    char* endptr = nullptr;
    long port_num = std::strtol(port, &endptr, 10);
    if (*endptr != '\\0' || port_num < 1 || port_num > 65535) {
        std::cerr << "Error: Invalid port number" << std::endl;
        return nullptr;
    }
    
    // Build connection string with SSL enforcement (Rules#5, #7)
    // sslmode=require ensures encrypted connection
    std::string conninfo = "host=" + std::string(host) +
                          " port=" + std::string(port) +
                          " dbname=" + std::string(dbname) +
                          " user=" + std::string(user) +
                          " password=" + std::string(password) +
                          " sslmode=require" +  // Enforce SSL/TLS
                          " connect_timeout=10";  // Prevent indefinite hangs
    
    // Create connection (Rules#14 - error handling)
    PGconn* conn = PQconnectdb(conninfo.c_str());
    
    // Clear connection string from memory to prevent credential leakage (Rules#1)
    conninfo.clear();
    
    // Check connection status (Rules#14)
    if (PQstatus(conn) != CONNECTION_OK) {
        std::cerr << "Connection failed: " << PQerrorMessage(conn) << std::endl;
        PQfinish(conn);
        return nullptr;
    }
    
    return conn;
}

// Test function
int main() {
    std::cout << "PostgreSQL Connection Test Cases:" << std::endl;
    std::cout << "==================================" << std::endl;
    
    // Test Case 1: Valid connection (requires env vars to be set)
    std::cout << "\\nTest 1: Attempting connection with environment variables..." << std::endl;
    PGconn* conn1 = createPostgresConnection();
    if (conn1) {
        std::cout << "Test 1 PASSED: Connection established successfully" << std::endl;
        std::cout << "Server version: " << PQserverVersion(conn1) << std::endl;
        PQfinish(conn1);
    } else {
        std::cout << "Test 1 FAILED: Could not establish connection" << std::endl;
    }
    
    // Test Case 2: Missing environment variable
    std::cout << "\\nTest 2: Testing with missing PGHOST..." << std::endl;
    const char* original_host = std::getenv("PGHOST");
    std::string saved_host = original_host ? original_host : "";
    unsetenv("PGHOST");
    PGconn* conn2 = createPostgresConnection();
    if (!conn2) {
        std::cout << "Test 2 PASSED: Correctly rejected missing parameter" << std::endl;
    } else {
        std::cout << "Test 2 FAILED: Should have rejected connection" << std::endl;
        PQfinish(conn2);
    }
    if (!saved_host.empty()) {
        setenv("PGHOST", saved_host.c_str(), 1);
    }
    
    // Test Case 3: Invalid port
    std::cout << "\\nTest 3: Testing with invalid port..." << std::endl;
    const char* original_port = std::getenv("PGPORT");
    std::string saved_port = original_port ? original_port : "";
    setenv("PGPORT", "invalid_port", 1);
    PGconn* conn3 = createPostgresConnection();
    if (!conn3) {
        std::cout << "Test 3 PASSED: Correctly rejected invalid port" << std::endl;
    } else {
        std::cout << "Test 3 FAILED: Should have rejected invalid port" << std::endl;
        PQfinish(conn3);
    }
    if (!saved_port.empty()) {
        setenv("PGPORT", saved_port.c_str(), 1);
    }
    
    // Test Case 4: Port out of range
    std::cout << "\\nTest 4: Testing with port out of range..." << std::endl;
    setenv("PGPORT", "99999", 1);
    PGconn* conn4 = createPostgresConnection();
    if (!conn4) {
        std::cout << "Test 4 PASSED: Correctly rejected out-of-range port" << std::endl;
    } else {
        std::cout << "Test 4 FAILED: Should have rejected out-of-range port" << std::endl;
        PQfinish(conn4);
    }
    if (!saved_port.empty()) {
        setenv("PGPORT", saved_port.c_str(), 1);
    }
    
    // Test Case 5: Valid connection parameters restored
    std::cout << "\\nTest 5: Retesting with restored valid parameters..." << std::endl;
    PGconn* conn5 = createPostgresConnection();
    if (conn5) {
        std::cout << "Test 5 PASSED: Connection re-established" << std::endl;
        PQfinish(conn5);
    } else {
        std::cout << "Test 5 FAILED: Could not re-establish connection" << std::endl;
    }
    
    return 0;
}
