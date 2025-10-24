#include <iostream>
#include <string>
#include <memory>
#include <pqxx/pqxx>

// Note: This code requires the libpqxx library.
// On Debian/Ubuntu: sudo apt-get install libpqxx-dev
// On macOS (Homebrew): brew install libpqxx
// Compile with: g++ your_file.cpp -o your_app -lpqxx -lpq

/**
 * Creates and returns a connection object for a Postgres database.
 * For security, this function attempts to connect using SSL.
 *
 * @param host The database server host.
 * @param port The database server port.
 * @param dbname The name of the database.
 * @param user The username for authentication.
 * @param password The password for authentication.
 * @return A unique_ptr to a pqxx::connection object, or nullptr if connection fails.
 */
std::unique_ptr<pqxx::connection> getPostgresConnection(
    const std::string& host,
    const std::string& port,
    const std::string& dbname,
    const std::string& user,
    const std::string& password) {
    
    // Build a secure connection string with SSL mode required
    std::string conn_string = "host=" + host +
                              " port=" + port +
                              " dbname=" + dbname +
                              " user=" + user +
                              " password=" + password +
                              " sslmode=require";

    try {
        auto conn = std::make_unique<pqxx::connection>(conn_string);
        if (conn->is_open()) {
            std::cout << "Connection to " << conn->dbname() << " established successfully." << std::endl;
            return conn;
        }
    } catch (const pqxx::sql_error& e) {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Query was: " << e.query() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Connection failed: " << e.what() << std::endl;
    }
    
    return nullptr;
}


int main() {
    // --- Test Cases ---
    // These are placeholder credentials and will likely fail to connect.
    // Replace with your actual database details to test a successful connection.

    // Test Case 1: Valid-looking but likely incorrect credentials
    std::cout << "--- Test Case 1 ---" << std::endl;
    auto conn1 = getPostgresConnection("localhost", "5432", "mydatabase", "myuser", "mypassword");
    if (conn1) {
        std::cout << "Test Case 1: Connection object returned." << std::endl;
        // Connection is automatically closed when conn1 goes out of scope.
    } else {
        std::cout << "Test Case 1: Failed to get connection object." << std::endl;
    }
    std::cout << std::endl;

    // Test Case 2: Invalid host
    std::cout << "--- Test Case 2 ---" << std::endl;
    auto conn2 = getPostgresConnection("invalid-host", "5432", "testdb", "user2", "pass2");
    if (conn2) {
        std::cout << "Test Case 2: Connection object returned." << std::endl;
    } else {
        std::cout << "Test Case 2: Failed to get connection object." << std::endl;
    }
    std::cout << std::endl;

    // Test Case 3: Invalid port
    std::cout << "--- Test Case 3 ---" << std::endl;
    auto conn3 = getPostgresConnection("localhost", "9999", "proddb", "user3", "pass3");
    if (conn3) {
        std::cout << "Test Case 3: Connection object returned." << std::endl;
    } else {
        std::cout << "Test Case 3: Failed to get connection object." << std::endl;
    }
    std::cout << std::endl;

    // Test Case 4: Non-existent database
    std::cout << "--- Test Case 4 ---" << std::endl;
    auto conn4 = getPostgresConnection("localhost", "5432", "nonexistentdb", "user4", "pass4");
    if (conn4) {
        std::cout << "Test Case 4: Connection object returned." << std::endl;
    } else {
        std::cout << "Test Case 4: Failed to get connection object." << std::endl;
    }
    std::cout << std::endl;

    // Test Case 5: Empty parameters
    std::cout << "--- Test Case 5 ---" << std::endl;
    auto conn5 = getPostgresConnection("", "", "", "", "");
    if (conn5) {
        std::cout << "Test Case 5: Connection object returned." << std::endl;
    } else {
        std::cout << "Test Case 5: Failed to get connection object." << std::endl;
    }
    std::cout << std::endl;

    return 0;
}