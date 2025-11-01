/*
 * Note: This code requires the libpqxx C++ library for PostgreSQL.
 * You must have the PostgreSQL development libraries and libpqxx installed.
 * 
 * On Debian/Ubuntu:
 * sudo apt-get install libpq-dev libpqxx-dev
 *
 * On RedHat/CentOS:
 * sudo yum install postgresql-devel libpqxx-devel
 *
 * Compilation command:
 * g++ -std=c++17 your_file_name.cpp -o Lse139 -lpqxx -lpq
 *
 * Before running, set the following environment variables:
 * DB_HOST: The database host (e.g., localhost)
 * DB_PORT: The database port (e.g., 5432)
 * DB_NAME: The database name
 * DB_USER: The database user
 * DB_PASSWORD: The database user's password
 *
 * Assumed SQL Schema:
 * CREATE TABLE users (
 *     user_id SERIAL PRIMARY KEY,
 *     username VARCHAR(50) UNIQUE NOT NULL,
 *     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
 * );
 */
#include <iostream>
#include <string>
#include <cstdlib> // For getenv
#include <pqxx/pqxx>
#include <vector>

class Lse139 {
public:
    /**
     * Checks if a user is a moderator by querying the database.
     * Credentials are read from environment variables.
     * Uses prepared statements (via exec_params) to prevent SQL injection.
     *
     * @param username The username to check.
     * @return True if the user is a moderator, false otherwise.
     */
    static bool isUserModerator(const std::string& username) {
        // Ensure username is not empty
        if (username.empty()) {
            return false;
        }

        try {
            // 1. Get credentials securely from environment variables
            const char* db_host = std::getenv("DB_HOST");
            const char* db_port = std::getenv("DB_PORT");
            const char* db_name = std::getenv("DB_NAME");
            const char* db_user = std::getenv("DB_USER");
            const char* db_password = std::getenv("DB_PASSWORD");

            if (!db_host || !db_port || !db_name || !db_user || !db_password) {
                std::cerr << "Error: Database environment variables are not set." << std::endl;
                return false;
            }

            std::string conn_string = "dbname=" + std::string(db_name) +
                                      " user=" + std::string(db_user) +
                                      " password=" + std::string(db_password) +
                                      " hostaddr=" + std::string(db_host) +
                                      " port=" + std::string(db_port);

            pqxx::connection C(conn_string);
            if (!C.is_open()) {
                std::cerr << "Can't open database" << std::endl;
                return false;
            }

            // 2. Use a transaction and parameterized query to prevent SQL injection
            pqxx::work W(C);
            std::string sql = "SELECT is_moderator FROM users WHERE username = $1";
            
            // 3. Execute query safely
            pqxx::result R = W.exec_params(sql, username);
            W.commit();

            // If a row is found, get the boolean value
            if (!R.empty()) {
                return R[0][0].as<bool>();
            }

        } catch (const std::exception &e) {
            // 4. Handle exceptions properly
            std::cerr << "Database error: " << e.what() << std::endl;
            return false;
        }

        return false;
    }
};

int main() {
    std::cout << "Running test cases..." << std::endl;
    std::cout << "Please ensure your database is running and environment variables are set." << std::endl;
    std::cout << "Assumed test data:" << std::endl;
    std::cout << " - user 'admin' is a moderator" << std::endl;
    std::cout << " - user 'bob' is NOT a moderator" << std::endl;
    std::cout << " - user 'charlie' does not exist" << std::endl;
    std::cout << "------------------------------------" << std::endl;
    
    std::vector<std::string> testUsers = {
        "admin",               // Should be true
        "bob",                 // Should be false
        "charlie",             // Non-existent user, should be false
        "'; DROP TABLE users; --", // Malicious input, should be false and safe
        ""                     // Empty input
    };

    for (const auto& user : testUsers) {
        bool result = Lse139::isUserModerator(user);
        std::cout << "Is user '" << user << "' a moderator? -> " << std::boolalpha << result << std::endl;
    }

    return 0;
}