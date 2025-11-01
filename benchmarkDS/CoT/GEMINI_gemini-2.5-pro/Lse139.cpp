#include <iostream>
#include <string>
#include <cstdlib> // For std::getenv
#include <stdexcept>

// This code requires the libpqxx library, the C++ API for PostgreSQL.
// On Debian/Ubuntu: sudo apt-get install libpqxx-dev
// On macOS (Homebrew): brew install libpqxx
// Compile with: g++ -o program your_file.cpp -lpqxx -lpq
#include <pqxx/pqxx>

/**
 * Connects to a PostgreSQL database using credentials from environment variables
 * and checks if a user is a moderator.
 *
 * Required Environment Variables:
 * DB_HOST: The database host (e.g., "localhost")
 * DB_PORT: The database port (e.g., "5432")
 * DB_NAME: The name of the database (e.g., "appdb")
 * DB_USER: The database username
 * DB_PASS: The database user's password
 *
 * Assumed table schema:
 * CREATE TABLE users (
 *     user_id SERIAL PRIMARY KEY,
 *     username VARCHAR(255) UNIQUE NOT NULL,
 *     is_moderator BOOLEAN NOT NULL DEFAULT FALSE
 * );
 *
 * @param userId The ID of the user to check.
 * @return true if the user is a moderator, false otherwise or if an error occurs.
 */
bool isUserModerator(int userId) {
    try {
        const char* host = std::getenv("DB_HOST");
        const char* port = std::getenv("DB_PORT");
        const char* dbname = std::getenv("DB_NAME");
        const char* user = std::getenv("DB_USER");
        const char* password = std::getenv("DB_PASS");

        if (!host || !port || !dbname || !user || !password) {
            std::cerr << "Error: Database environment variables are not fully set." << std::endl;
            return false;
        }

        std::string conn_str = "host=" + std::string(host) +
                               " port=" + std::string(port) +
                               " dbname=" + std::string(dbname) +
                               " user=" + std::string(user) +
                               " password=" + std::string(password);
        
        pqxx::connection conn(conn_str);
        if (!conn.is_open()) {
            std::cerr << "Can't open database." << std::endl;
            return false;
        }

        pqxx::work txn(conn);
        std::string sql = "SELECT is_moderator FROM users WHERE user_id = $1;";
        
        // Use exec_params for parameterized queries to prevent SQL injection
        pqxx::result res = txn.exec_params(sql, userId);

        // Commit the transaction (it's a read-only query but good practice)
        txn.commit();
        
        if (res.empty()) {
            return false; // User not found
        }
        
        return res[0][0].as<bool>();

    } catch (const std::exception &e) {
        std::cerr << "Database Error: " << e.what() << std::endl;
        return false;
    }
}

class Lse139 {
public:
    static void main() {
        std::cout << "--- C++ Test Cases ---" << std::endl;
        std::cout << "Note: This requires a running PostgreSQL database, 'libpqxx' library, and correctly set environment variables." << std::endl;
        std::cout << "See comments in the isUserModerator function for setup details." << std::endl;
        std::cout << std::boolalpha; // Print booleans as "true" or "false"

        // Test cases (these will only work if you have a corresponding database setup)
        // Assume user 1 is a moderator, user 2 is not, and user 99 does not exist.
        int moderatorUserId = 1;
        int regularUserId = 2;
        int nonExistentUserId = 99;
        int anotherModeratorId = 3;
        int anotherRegularId = 4;

        std::cout << "Is user " << moderatorUserId << " a moderator? " << isUserModerator(moderatorUserId) << std::endl;
        std::cout << "Is user " << regularUserId << " a moderator? " << isUserModerator(regularUserId) << std::endl;
        std::cout << "Is user " << nonExistentUserId << " a moderator? " << isUserModerator(nonExistentUserId) << std::endl;
        std::cout << "Is user " << anotherModeratorId << " a moderator? " << isUserModerator(anotherModeratorId) << std::endl;
        std::cout << "Is user " << anotherRegularId << " a moderator? " << isUserModerator(anotherRegularId) << std::endl;
    }
};

int main() {
    Lse139::main();
    return 0;
}