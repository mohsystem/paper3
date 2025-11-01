#include <iostream>
#include <string>
#include <cstdlib> // For getenv
#include <mysql/mysql.h>
#include <regex>

// Rule #1: Validate username format: 3-20 characters, alphanumeric and underscores.
const std::regex USERNAME_REGEX("^[a-zA-Z0-9_]{3,20}$");

/**
 * Connects to a MySQL database and checks if a username exists.
 * This function uses environment variables for database credentials.
 * export DB_HOST="localhost"
 * export DB_USER="youruser"
 * export DB_PASS="yourpassword"
 * export DB_NAME="testdb"
 *
 * @param username The username to check.
 * @return true if the username exists, false otherwise.
 */
bool usernameExists(const std::string& username) {
    // Rule #1: Ensure all input is validated.
    if (!std::regex_match(username, USERNAME_REGEX)) {
        std::cerr << "Invalid username format." << std::endl;
        return false;
    }

    // Rule #5 & #6: Use environment variables for credentials.
    const char* db_host = std::getenv("DB_HOST");
    const char* db_user = std::getenv("DB_USER");
    const char* db_pass = std::getenv("DB_PASS");
    const char* db_name = std::getenv("DB_NAME");

    if (!db_host || !db_user || !db_pass || !db_name) {
        std::cerr << "Database environment variables (DB_HOST, DB_USER, DB_PASS, DB_NAME) are not set." << std::endl;
        return false;
    }

    MYSQL* conn = mysql_init(NULL);
    if (conn == NULL) {
        std::cerr << "mysql_init() failed" << std::endl;
        return false;
    }

    if (mysql_real_connect(conn, db_host, db_user, db_pass, db_name, 0, NULL, 0) == NULL) {
        std::cerr << "mysql_real_connect() failed: " << mysql_error(conn) << std::endl;
        mysql_close(conn);
        return false;
    }

    bool exists = false;
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        std::cerr << "mysql_stmt_init() failed" << std::endl;
        mysql_close(conn);
        return false;
    }

    // Using a prepared statement to prevent SQL injection (CWE-89).
    std::string query = "SELECT username FROM users WHERE username = ? LIMIT 1";
    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << std::endl;
        goto cleanup;
    }

    MYSQL_BIND bind[1];
    memset(bind, 0, sizeof(bind));
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char*)username.c_str();
    bind[0].buffer_length = username.length();

    if (mysql_stmt_bind_param(stmt, bind)) {
        std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
        goto cleanup;
    }

    if (mysql_stmt_execute(stmt)) {
        std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
        goto cleanup;
    }

    // Check if any rows were returned
    if (mysql_stmt_store_result(stmt)) {
         std::cerr << "mysql_stmt_store_result() failed: " << mysql_stmt_error(stmt) << std::endl;
         goto cleanup;
    }

    if (mysql_stmt_num_rows(stmt) > 0) {
        exists = true;
    }

cleanup:
    mysql_stmt_close(stmt);
    mysql_close(conn);
    return exists;
}

// Helper to set up the database.
void setupDatabaseForTesting() {
    const char* db_host = std::getenv("DB_HOST");
    const char* db_user = std::getenv("DB_USER");
    const char* db_pass = std::getenv("DB_PASS");
    const char* db_name = std::getenv("DB_NAME");

    if (!db_host || !db_user || !db_pass || !db_name) return;

    MYSQL* conn = mysql_init(NULL);
    if (!conn) return;

    if (mysql_real_connect(conn, db_host, db_user, db_pass, db_name, 0, NULL, 0)) {
        const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users (id INT AUTO_INCREMENT PRIMARY KEY, username VARCHAR(255) NOT NULL UNIQUE);";
        const char* insertSQL = "INSERT IGNORE INTO users (username) VALUES ('alice'), ('bob');";
        mysql_query(conn, createTableSQL);
        mysql_query(conn, insertSQL);
        std::cout << "Database setup for testing complete." << std::endl;
    }
    mysql_close(conn);
}


int main() {
    std::cout << "--- Running C++ Username Existence Check ---" << std::endl;
    std::cout << "This test requires a MySQL database and the following environment variables:" << std::endl;
    std::cout << "export DB_HOST=\"localhost\"" << std::endl;
    std::cout << "export DB_USER=\"youruser\"" << std::endl;
    std::cout << "export DB_PASS=\"yourpassword\"" << std::endl;
    std::cout << "export DB_NAME=\"testdb\"" << std::endl;
    std::cout << "Compile with: g++ your_file.cpp -o your_app -lmysqlclient" << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
    
    setupDatabaseForTesting();

    std::string testUsernames[] = {
        "alice",        // Test case 1: Existing user
        "charlie",      // Test case 2: Non-existent user
        "dave_has_a_very_long_name_that_is_invalid", // Test case 3: Invalid (too long)
        "eve-vil",      // Test case 4: Invalid (contains '-')
        ""              // Test case 5: Invalid (empty)
    };

    for (int i = 0; i < 5; ++i) {
        const std::string& username = testUsernames[i];
        std::cout << "Test Case " << i + 1 << ": Checking for username '" << username << "'..." << std::endl;
        bool exists = usernameExists(username);
        std::cout << "Result: Username '" << username << "' " << (exists ? "exists" : "does not exist or is invalid") << ".\n" << std::endl;
    }

    return 0;
}