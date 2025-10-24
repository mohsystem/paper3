#include <iostream>
#include <string>
#include <pqxx/pqxx>
#include <memory>

/*
 * To compile and run this code, you need the libpqxx library and PostgreSQL's C client library (libpq).
 * On Debian/Ubuntu: sudo apt-get install libpqxx-dev
 * On RedHat/CentOS: sudo dnf install libpqxx-devel
 *
 * Example compilation:
 * g++ -o task74 Task74.cpp -lpqxx -lpq
 */

/**
 * Establishes a connection to a PostgreSQL database.
 *
 * @param conn_string The connection string (e.g., "dbname=testdb user=postgres password=secret ...").
 * @return A unique_ptr to a pqxx::connection object, or an empty unique_ptr if the connection fails.
 *
 * Security Note: In a real application, the connection string should be built from
 * securely sourced credentials and not hardcoded. Always use prepared statements
 * with the returned connection (e.g., conn->prepare()) to prevent SQL injection.
 */
std::unique_ptr<pqxx::connection> connectToPostgres(const std::string& conn_string) {
    try {
        return std::make_unique<pqxx::connection>(conn_string);
    } catch (const pqxx::broken_connection &e) {
        std::cerr << "Connection Error: " << e.what() << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    return nullptr;
}

int main() {
    // --- IMPORTANT ---
    // Replace these placeholder values with your actual PostgreSQL database details.
    // For the test cases to work as expected, you need a running PostgreSQL server.
    const std::string HOST = "localhost";
    const std::string PORT = "5432";
    const std::string DBNAME = "your_db";
    const std::string USER = "your_user";
    const std::string PASSWORD = "your_password";

    // Test Case 1: Successful Connection
    std::cout << "--- Test Case 1: Attempting a successful connection... ---" << std::endl;
    std::string success_conn_str = "dbname=" + DBNAME + " user=" + USER + " password=" + PASSWORD + " host=" + HOST + " port=" + PORT;
    auto conn1 = connectToPostgres(success_conn_str);
    if (conn1 && conn1->is_open()) {
        std::cout << "Test Case 1: Connection successful!" << std::endl;
        conn1->disconnect();
        std::cout << "Test Case 1: Connection closed." << std::endl;
    } else {
        std::cout << "Test Case 1: Connection failed. Please check your credentials and database status." << std::endl;
    }
    std::cout << std::endl;

    // Test Case 2: Incorrect Password
    std::cout << "--- Test Case 2: Attempting connection with a wrong password... ---" << std::endl;
    std::string bad_pass_str = "dbname=" + DBNAME + " user=" + USER + " password=wrong_password host=" + HOST + " port=" + PORT;
    auto conn2 = connectToPostgres(bad_pass_str);
    if (!conn2) {
        std::cout << "Test Case 2: Connection failed as expected." << std::endl;
    } else {
        std::cout << "Test Case 2: Connection unexpectedly succeeded." << std::endl;
        conn2->disconnect();
    }
    std::cout << std::endl;

    // Test Case 3: Incorrect Database Name
    std::cout << "--- Test Case 3: Attempting connection to a non-existent database... ---" << std::endl;
    std::string bad_db_str = "dbname=non_existent_db user=" + USER + " password=" + PASSWORD + " host=" + HOST + " port=" + PORT;
    auto conn3 = connectToPostgres(bad_db_str);
    if (!conn3) {
        std::cout << "Test Case 3: Connection failed as expected." << std::endl;
    } else {
        std::cout << "Test Case 3: Connection unexpectedly succeeded." << std::endl;
        conn3->disconnect();
    }
    std::cout << std::endl;

    // Test Case 4: Incorrect User
    std::cout << "--- Test Case 4: Attempting connection with a wrong user... ---" << std::endl;
    std::string bad_user_str = "dbname=" + DBNAME + " user=wrong_user password=" + PASSWORD + " host=" + HOST + " port=" + PORT;
    auto conn4 = connectToPostgres(bad_user_str);
    if (!conn4) {
        std::cout << "Test Case 4: Connection failed as expected." << std::endl;
    } else {
        std::cout << "Test Case 4: Connection unexpectedly succeeded." << std::endl;
        conn4->disconnect();
    }
    std::cout << std::endl;
    
    // Test Case 5: Incorrect Host
    std::cout << "--- Test Case 5: Attempting connection to a non-existent host... ---" << std::endl;
    std::string bad_host_str = "dbname=" + DBNAME + " user=" + USER + " password=" + PASSWORD + " host=nonexistenthost port=" + PORT;
    auto conn5 = connectToPostgres(bad_host_str);
    if (!conn5) {
        std::cout << "Test Case 5: Connection failed as expected." << std::endl;
    } else {
        std::cout << "Test Case 5: Connection unexpectedly succeeded." << std::endl;
        conn5->disconnect();
    }
    std::cout << std::endl;

    return 0;
}