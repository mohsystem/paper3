#include <iostream>
#include <string>
#include <memory>
#include <pqxx/pqxx>

/**
 * Creates and returns a unique_ptr to a pqxx::connection object.
 * Note: You need to have libpqxx and libpq installed and link against them.
 * Example compile command: g++ -std=c++17 your_file.cpp -o your_app -lpqxx -lpq
 *
 * @param host     The database server host.
 * @param port     The database server port as a string.
 * @param dbName   The name of the database.
 * @param user     The username for authentication.
 * @param password The password for authentication.
 * @return A unique_ptr to a connection object, or nullptr if connection fails.
 */
std::unique_ptr<pqxx::connection> createConnection(
    const std::string& host, 
    const std::string& port, 
    const std::string& dbName, 
    const std::string& user, 
    const std::string& password) 
{
    try {
        std::string conn_str = "host=" + host + " port=" + port + " dbname=" + dbName + 
                               " user=" + user + " password=" + password;
        auto conn = std::make_unique<pqxx::connection>(conn_str);
        return conn;
    } catch (const pqxx::sql_error &e) {
        // In a real application, you would log this error.
        // std::cerr << "SQL error: " << e.what() << std::endl;
        return nullptr;
    } catch (const std::exception &e) {
        // std::cerr << "Connection error: " << e.what() << std::endl;
        return nullptr;
    }
}

void runTestCase(
    int caseNum, 
    const std::string& host, 
    const std::string& port, 
    const std::string& dbName, 
    const std::string& user, 
    const std::string& password)
{
    std::cout << "--- Test Case " << caseNum << " ---" << std::endl;
    std::cout << "Attempting to connect to postgresql://" << user << "@" << host << ":" << port << "/" << dbName << std::endl;

    auto conn = createConnection(host, port, dbName, user, password);
    if (conn && conn->is_open()) {
        std::cout << "Connection successful!" << std::endl;
        conn->disconnect();
        std::cout << "Connection closed." << std::endl;
    } else {
        std::cout << "Connection failed." << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    // --- Test Cases ---

    // Test Case 1: Standard localhost connection (likely to fail without a running DB and correct credentials)
    runTestCase(1, "localhost", "5432", "testdb", "user", "password");

    // Test Case 2: Using IP address for localhost
    runTestCase(2, "127.0.0.1", "5432", "postgres", "postgres", "admin123");

    // Test Case 3: Different database and user
    runTestCase(3, "localhost", "5432", "mydatabase", "dbuser", "secret");

    // Test Case 4: Invalid host to simulate connection failure
    runTestCase(4, "nonexistent.host.com", "5432", "testdb", "user", "password");

    // Test Case 5: Invalid port
    runTestCase(5, "localhost", "9999", "testdb", "user", "password");

    return 0;
}