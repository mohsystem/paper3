
#include <iostream>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <regex>
#include <memory>
#include <cstring>
#include <mysql/mysql.h>
#include <microhttpd.h>

// Security: Never hard-code credentials. Read from environment variables or secret manager.
// Rules#3: Avoid hard-coded passwords or secrets

// Returns database server name from environment variable with validation
// Rules#2: Input validation - check type, length, range
std::string getServerName() {
    const char* server = std::getenv("DB_SERVER");
    if (server == nullptr || std::strlen(server) == 0) {
        throw std::runtime_error("DB_SERVER environment variable not set");
    }
    
    std::string serverStr(server);
    
    // Validate server name format (alphanumeric, dots, hyphens only)
    std::regex validPattern("^[a-zA-Z0-9.-]+$");
    if (!std::regex_match(serverStr, validPattern)) {
        throw std::invalid_argument("Invalid server name format");
    }
    
    if (serverStr.length() > 253) {
        throw std::invalid_argument("Server name too long");
    }
    
    return serverStr;
}

// Returns database name from environment variable with validation
std::string getDatabaseName() {
    const char* dbName = std::getenv("DB_NAME");
    if (dbName == nullptr || std::strlen(dbName) == 0) {
        throw std::runtime_error("DB_NAME environment variable not set");
    }
    
    std::string dbNameStr(dbName);
    
    // Validate database name (alphanumeric and underscore only)
    std::regex validPattern("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(dbNameStr, validPattern)) {
        throw std::invalid_argument("Invalid database name format");
    }
    
    if (dbNameStr.length() > 64) {
        throw std::invalid_argument("Database name too long");
    }
    
    return dbNameStr;
}

// Returns database user from environment variable with validation
std::string getDatabaseUser() {
    const char* user = std::getenv("DB_USER");
    if (user == nullptr || std::strlen(user) == 0) {
        throw std::runtime_error("DB_USER environment variable not set");
    }
    
    std::string userStr(user);
    
    // Validate username format
    std::regex validPattern("^[a-zA-Z0-9_]+$");
    if (!std::regex_match(userStr, validPattern)) {
        throw std::invalid_argument("Invalid username format");
    }
    
    if (userStr.length() > 32) {
        throw std::invalid_argument("Username too long");
    }
    
    return userStr;
}

// Returns database password from environment variable
// Security: Never log or expose passwords (Rules#10)
std::string getDatabasePassword() {
    const char* password = std::getenv("DB_PASSWORD");
    if (password == nullptr || std::strlen(password) == 0) {
        throw std::runtime_error("DB_PASSWORD environment variable not set");
    }
    
    std::string passwordStr(password);
    
    // Length validation only - don't restrict password characters
    if (passwordStr.length() > 128) {
        throw std::invalid_argument("Password too long");
    }
    
    return passwordStr;
}

// RAII wrapper for MySQL connection
// Rules#8: Memory safety with proper cleanup
class DatabaseConnection {
private:
    MYSQL* conn;
    
public:
    DatabaseConnection() : conn(nullptr) {
        try {
            std::string server = getServerName();
            std::string dbName = getDatabaseName();
            std::string user = getDatabaseUser();
            std::string password = getDatabasePassword();
            
            // Initialize MySQL connection
            conn = mysql_init(nullptr);
            if (conn == nullptr) {
                throw std::runtime_error("MySQL initialization failed");
            }
            
            // Rules#6: Enforce SSL/TLS for database connections
            mysql_ssl_set(conn, nullptr, nullptr, nullptr, nullptr, nullptr);
            unsigned int ssl_mode = SSL_MODE_REQUIRED;
            mysql_options(conn, MYSQL_OPT_SSL_MODE, &ssl_mode);
            
            // Establish connection
            if (mysql_real_connect(conn, server.c_str(), user.c_str(), 
                                  password.c_str(), dbName.c_str(), 
                                  3306, nullptr, CLIENT_SSL) == nullptr) {
                // Rules#11: Generic error message
                std::string error = "Database connection failed";
                mysql_close(conn);
                conn = nullptr;
                throw std::runtime_error(error);
            }
            
        } catch (...) {
            if (conn != nullptr) {
                mysql_close(conn);
                conn = nullptr;
            }
            throw;
        }
    }
    
    ~DatabaseConnection() {
        if (conn != nullptr) {
            mysql_close(conn);
            conn = nullptr;
        }
    }
    
    MYSQL* get() { return conn; }
    bool isValid() const { return conn != nullptr; }
};

// Returns a database connection using validated configuration
std::unique_ptr<DatabaseConnection> getDatabaseConnection() {
    return std::make_unique<DatabaseConnection>();
}

// Web app request handler
static int requestHandler(void* cls, struct MHD_Connection* connection,
                         const char* url, const char* method,
                         const char* version, const char* upload_data,
                         size_t* upload_data_size, void** con_cls) {
    // Rules#2: Validate HTTP method
    if (std::strcmp(method, "GET") != 0) {
        return MHD_NO;
    }
    
    const char* response = "Web application running";
    struct MHD_Response* mhd_response = MHD_create_response_from_buffer(
        std::strlen(response), (void*)response, MHD_RESPMEM_PERSISTENT);
    
    MHD_add_response_header(mhd_response, "Content-Type", "text/plain; charset=utf-8");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, mhd_response);
    MHD_destroy_response(mhd_response);
    
    return ret;
}

int main() {
    std::cout << "=== Lse141 Web App Test Cases ===" << std::endl << std::endl;
    
    // Test Case 1: Missing environment variables
    std::cout << "Test 1: Missing environment variables" << std::endl;
    try {
        getServerName();
        std::cout << "FAIL: Should have thrown exception" << std::endl;
    } catch (const std::runtime_error& e) {
        std::cout << "PASS: Correctly detected missing DB_SERVER" << std::endl;
    }
    
    // Test Case 2: Valid configuration check
    std::cout << "\\nTest 2: Valid configuration check" << std::endl;
    std::cout << "Note: Set environment variables DB_SERVER, DB_NAME, DB_USER, DB_PASSWORD before running" << std::endl;
    
    // Test Case 3: Invalid server name format
    std::cout << "\\nTest 3: Server name validation" << std::endl;
    setenv("DB_SERVER", "invalid;server", 1);
    try {
        getServerName();
        std::cout << "FAIL: Should have rejected invalid server name" << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "PASS: Server name validation works" << std::endl;
    }
    unsetenv("DB_SERVER");
    
    // Test Case 4: Start web server
    std::cout << "\\nTest 4: Starting web server" << std::endl;
    struct MHD_Daemon* daemon = MHD_start_daemon(
        MHD_USE_SELECT_INTERNALLY, 8080, nullptr, nullptr,
        &requestHandler, nullptr, MHD_OPTION_END);
    
    if (daemon != nullptr) {
        std::cout << "PASS: Web server created on port 8080" << std::endl;
        MHD_stop_daemon(daemon);
    } else {
        std::cout << "FAIL: Could not create web server" << std::endl;
    }
    
    // Test Case 5: Database connection (requires valid env vars)
    std::cout << "\\nTest 5: Database connection" << std::endl;
    try {
        auto conn = getDatabaseConnection();
        if (conn && conn->isValid()) {
            std::cout << "PASS: Database connection successful" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cout << "INFO: Database connection requires valid environment variables" << std::endl;
    }
    
    std::cout << "\\n=== Tests Complete ===" << std::endl;
    
    return 0;
}
