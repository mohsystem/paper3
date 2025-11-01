#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <cstdlib> // For std::getenv
#include <pqxx/pqxx>

// To compile this code, you need the libpqxx library.
// On Debian/Ubuntu: sudo apt-get install libpqxx-dev
// Compile command: g++ -std=c++11 -o moderator_check moderator_check.cpp -lpqxx -lpq
//
// Before running, set the following environment variables:
// export DB_HOST=localhost
// export DB_PORT=5432
// export DB_NAME=yourdbname
// export DB_USER=youruser
// export DB_PASS=yourpassword
//
// And create the required table and data in your PostgreSQL database:
// CREATE TABLE users (
//     id SERIAL PRIMARY KEY,
//     username VARCHAR(50) UNIQUE NOT NULL,
//     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
// );
// INSERT INTO users (username, is_moderator) VALUES ('alice', TRUE);
// INSERT INTO users (username, is_moderator) VALUES ('bob', FALSE);

/**
 * Checks if a user is a moderator by querying a database.
 * Credentials and connection details are read from environment variables.
 * Uses parameterized queries to prevent SQL injection.
 *
 * @param username The username to check.
 * @return true if the user exists and is a moderator, false otherwise.
 */
bool isUserModerator(const std::string& username) {
    // Rule #1: Ensure all input is validated.
    if (username.empty()) {
        std::cerr << "Error: Username cannot be empty." << std::endl;
        return false;
    }

    // Rules #6, #7: Use environment variables for credentials.
    const char* db_host = std::getenv("DB_HOST");
    const char* db_port = std::getenv("DB_PORT");
    const char* db_name = std::getenv("DB_NAME");
    const char* db_user = std::getenv("DB_USER");
    const char* db_pass = std::getenv("DB_PASS");

    if (!db_host || !db_port || !db_name || !db_user || !db_pass) {
        std::cerr << "Error: Database environment variables not set." << std::endl;
        std::cerr << "Please set DB_HOST, DB_PORT, DB_NAME, DB_USER, and DB_PASS." << std::endl;
        return false;
    }

    try {
        std::string conn_string = "host=" + std::string(db_host) +
                                  " port=" + std::string(db_port) +
                                  " dbname=" + std::string(db_name) +
                                  " user=" + std::string(db_user) +
                                  " password=" + std::string(db_pass);

        pqxx::connection conn(conn_string);
        pqxx::work txn(conn);

        // The query uses a placeholder ($1) for parameterization to prevent SQL injection.
        const std::string sql = "SELECT is_moderator FROM users WHERE username = $1";
        
        // Execute query with parameters.
        pqxx::result r = txn.exec_params(sql, username);
        
        txn.commit();
        
        // If no rows are returned, user not found or not a moderator.
        if (r.empty()) {
            return false;
        }
        
        // If a row is found, get the boolean value.
        return r[0][0].as<bool>();

    } catch (const std::exception &e) {
        // In a production environment, log this error to a secure logging system.
        std::cerr << "Database query failed: " << e.what() << std::endl;
        return false; // Fail closed on any exception.
    }
}

int main() {
    std::cout << "Running moderator check tests..." << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "This test assumes a PostgreSQL database is running and" << std::endl;
    std::cout << "the required environment variables and table are set up." << std::endl;
    std::cout << "---------------------------------" << std::endl;

    // 5 Test cases
    std::vector<std::string> test_users = {
        "alice",          // Expected: true (existing moderator)
        "bob",            // Expected: false (existing non-moderator)
        "eve",            // Expected: false (non-existent user)
        "' OR '1'='1",    // Expected: false (SQL injection attempt)
        ""                // Expected: false (invalid input)
    };

    for (const auto& user : test_users) {
        std::cout << "Checking if user '" << (user.empty() ? "<empty>" : user) << "' is a moderator... "
                  << std::boolalpha << isUserModerator(user) << std::endl;
    }

    return 0;
}