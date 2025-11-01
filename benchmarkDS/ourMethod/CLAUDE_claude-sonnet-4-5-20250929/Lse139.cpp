
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <mysql/mysql.h>
#include <cstring>

/**
 * Check if a user is a moderator by querying the database.
 * Uses environment variables for database credentials.
 * 
 * @param username The username to check
 * @return true if user is a moderator, false otherwise
 * @throws std::invalid_argument if username is invalid
 * @throws std::runtime_error if database credentials are not set
 */
bool isUserModerator(const std::string& username) {
    // Validate input (Rules#8)
    if (username.empty()) {
        throw std::invalid_argument("Username cannot be empty");
    }
    
    // Limit username length to prevent potential attacks (Rules#8)
    if (username.length() > 255) {
        throw std::invalid_argument("Username exceeds maximum length");
    }
    
    // Retrieve database credentials from environment variables (Rules#4, Rules#5)
    const char* dbHost = std::getenv("DB_HOST");
    const char* dbName = std::getenv("DB_NAME");
    const char* dbUser = std::getenv("DB_USER");
    const char* dbPassword = std::getenv("DB_PASSWORD");
    const char* dbPortStr = std::getenv("DB_PORT");
    
    // Validate that credentials are available (Rules#8)
    if (!dbHost || !dbName || !dbUser || !dbPassword) {
        throw std::runtime_error("Database credentials not found in environment variables");
    }
    
    unsigned int dbPort = dbPortStr ? std::atoi(dbPortStr) : 3306;
    
    // Initialize MySQL connection
    MYSQL* conn = mysql_init(nullptr);
    if (!conn) {
        std::cerr << "MySQL initialization failed" << std::endl;
        return false;
    }
    
    // Establish database connection (Rules#11)
    if (!mysql_real_connect(conn, dbHost, dbUser, dbPassword, dbName, dbPort, nullptr, 0)) {
        std::cerr << "Database connection failed" << std::endl;
        mysql_close(conn);
        return false;
    }
    
    // Prepare statement to prevent SQL injection (Rules#1)
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) {
        std::cerr << "Statement initialization failed" << std::endl;
        mysql_close(conn);
        return false;
    }
    
    // Use parameterized query (Rules#1)
    const char* query = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
    
    if (mysql_stmt_prepare(stmt, query, std::strlen(query)) != 0) {
        std::cerr << "Statement preparation failed" << std::endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    // Bind parameter (Rules#1, Rules#8)
    MYSQL_BIND bind[1];
    std::memset(bind, 0, sizeof(bind));
    
    unsigned long usernameLen = username.length();
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = const_cast<char*>(username.c_str());
    bind[0].buffer_length = username.length();
    bind[0].length = &usernameLen;
    
    if (mysql_stmt_bind_param(stmt, bind) != 0) {
        std::cerr << "Parameter binding failed" << std::endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    // Execute query
    if (mysql_stmt_execute(stmt) != 0) {
        std::cerr << "Query execution failed" << std::endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    // Bind result
    MYSQL_BIND resultBind[1];
    std::memset(resultBind, 0, sizeof(resultBind));
    
    bool isModerator = false;
    my_bool isNull = 0;
    
    resultBind[0].buffer_type = MYSQL_TYPE_TINY;
    resultBind[0].buffer = &isModerator;
    resultBind[0].is_null = &isNull;
    
    if (mysql_stmt_bind_result(stmt, resultBind) != 0) {
        std::cerr << "Result binding failed" << std::endl;
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
    
    // Fetch result
    int fetchResult = mysql_stmt_fetch(stmt);
    bool result = false;
    
    if (fetchResult == 0 && !isNull) {
        result = isModerator;
    }
    
    // Clean up resources (Rules#11)
    mysql_stmt_close(stmt);
    mysql_close(conn);
    
    return result;
}

int main() {
    // For testing purposes only - set environment variables
    // In production, these should be actual environment variables
    #ifdef _WIN32
    _putenv_s("DB_HOST", "localhost");
    _putenv_s("DB_NAME", "testdb");
    _putenv_s("DB_USER", "testuser");
    _putenv_s("DB_PASSWORD", "testpass");
    _putenv_s("DB_PORT", "3306");
    #else
    setenv("DB_HOST", "localhost", 1);
    setenv("DB_NAME", "testdb", 1);
    setenv("DB_USER", "testuser", 1);
    setenv("DB_PASSWORD", "testpass", 1);
    setenv("DB_PORT", "3306", 1);
    #endif
    
    // Test case 1: Valid moderator user
    std::cout << "Test 1 - Valid moderator: " << (isUserModerator("admin_user") ? "true" : "false") << std::endl;
    
    // Test case 2: Valid non-moderator user
    std::cout << "Test 2 - Regular user: " << (isUserModerator("regular_user") ? "true" : "false") << std::endl;
    
    // Test case 3: Non-existent user
    std::cout << "Test 3 - Non-existent user: " << (isUserModerator("nonexistent") ? "true" : "false") << std::endl;
    
    // Test case 4: Empty username (should throw exception)
    try {
        isUserModerator("");
    } catch (const std::invalid_argument& e) {
        std::cout << "Test 4 - Empty username: Exception caught as expected" << std::endl;
    }
    
    // Test case 5: Long username test
    std::cout << "Test 5 - Normal username: " << (isUserModerator("test_user") ? "true" : "false") << std::endl;
    
    return 0;
}
