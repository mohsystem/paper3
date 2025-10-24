#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstdlib> // For std::getenv

/*
 * To compile and run this code, you need the libpqxx library.
 * On Debian/Ubuntu: sudo apt-get install libpqxx-dev
 * On macOS (Homebrew): brew install libpqxx
 *
 * Compile command:
 * g++ -std=c++17 -o task74 Task74.cpp -lpqxx -lpq
 *
 * Before running, set the following environment variables:
 * export PG_HOST="localhost"
 * export PG_PORT="5432"
 * export PG_DBNAME="testdb"
 * export PG_USER="testuser"
 * export PG_PASSWORD="testpassword"
 */

#include <pqxx/pqxx>

/**
 * Creates and returns a connection object for a PostgreSQL database.
 *
 * @param host     The database server host.
 * @param port     The database server port.
 * @param dbname   The name of the database.
 * @param user     The username for authentication.
 * @param password The password for authentication.
 * @return A unique_ptr to a pqxx::connection object, or nullptr if connection fails.
 */
std::unique_ptr<pqxx::connection> getConnection(
    const std::string& host,
    const std::string& port,
    const std::string& dbname,
    const std::string& user,
    const std::string& password)
{
    // Rule #13: Avoid hardcoding credentials. Credentials are passed as parameters.
    if (host.empty() || port.empty() || dbname.empty() || user.empty() || password.empty()) {
        std::cerr << "Error: All connection parameters must be provided." << std::endl;
        return nullptr;
    }

    // Construct the connection string. Using sslmode=require is a security best practice.
    // Rule #4, #5: Enforce SSL usage.
    std::string conn_string = "host=" + host +
                              " port=" + port +
                              " dbname=" + dbname +
                              " user=" + user +
                              " password=" + password +
                              " sslmode=require";

    try {
        auto conn = std::make_unique<pqxx::connection>(conn_string);
        if (conn->is_open()) {
            std::cout << "Connection to " << dbname << " established successfully." << std::endl;
            return conn;
        }
    } catch (const pqxx::sql_error& e) {
        // Rule #14: Handle exceptions gracefully without leaking sensitive info.
        std::cerr << "Connection Failed: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    
    return nullptr;
}

void run_test(const std::string& test_name,
              const std::string& host, const std::string& port, const std::string& dbname,
              const std::string& user, const std::string& password)
{
    std::cout << "\n[" << test_name << "]" << std::endl;
    if (host.empty()) {
        std::cout << "Skipping " << test_name << ": Environment variables not set." << std::endl;
        return;
    }
    std::unique_ptr<pqxx::connection> conn = getConnection(host, port, dbname, user, password);
    if (conn && conn->is_open()) {
        std::cout << "Connection object is valid." << std::endl;
        conn->disconnect();
    } else {
        std::cout << "Failed to get connection object as expected or an error occurred." << std::endl;
    }
}

int main() {
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Rule #12: Use environment variables to manage sensitive credentials
    const char* pg_host_cstr = std::getenv("PG_HOST");
    const char* pg_port_cstr = std::getenv("PG_PORT");
    const char* pg_dbname_cstr = std::getenv("PG_DBNAME");
    const char* pg_user_cstr = std::getenv("PG_USER");
    const char* pg_password_cstr = std::getenv("PG_PASSWORD");

    std::string db_host = pg_host_cstr ? pg_host_cstr : "";
    std::string db_port = pg_port_cstr ? pg_port_cstr : "";
    std::string db_name = pg_dbname_cstr ? pg_dbname_cstr : "";
    std::string db_user = pg_user_cstr ? pg_user_cstr : "";
    std::string db_password = pg_password_cstr ? pg_password_cstr : "";

    // Test Case 1: Successful connection using environment variables
    run_test("Test Case 1: Successful Connection", db_host, db_port, db_name, db_user, db_password);

    // Test Case 2: Connection with a wrong password
    run_test("Test Case 2: Wrong Password", db_host, db_port, db_name, db_user, "wrongpassword");

    // Test Case 3: Connection to a non-existent database
    run_test("Test Case 3: Non-existent Database", db_host, db_port, "nonexistentdb", db_user, db_password);

    // Test Case 4: Connection to a wrong host
    run_test("Test Case 4: Wrong Host", "127.0.0.99", db_port, db_name, db_user, db_password);

    // Test Case 5: Connection with an empty user
    run_test("Test Case 5: Empty User", db_host, db_port, db_name, "", db_password);

    return 0;
}