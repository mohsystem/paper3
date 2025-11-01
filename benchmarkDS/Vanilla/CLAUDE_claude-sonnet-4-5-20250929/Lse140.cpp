
#include <iostream>
#include <string>
#include <mysql/mysql.h>

bool checkUsernameExists(const std::string& username, const std::string& host, 
                         const std::string& user, const std::string& password, 
                         const std::string& database) {
    MYSQL* conn = nullptr;
    MYSQL_STMT* stmt = nullptr;
    MYSQL_BIND bind[1];
    MYSQL_BIND result[1];
    int count = 0;
    
    try {
        // Initialize MySQL connection
        conn = mysql_init(nullptr);
        if (conn == nullptr) {
            std::cerr << "mysql_init() failed" << std::endl;
            return false;
        }
        
        // Connect to the database
        if (mysql_real_connect(conn, host.c_str(), user.c_str(), password.c_str(),
                               database.c_str(), 0, nullptr, 0) == nullptr) {
            std::cerr << "mysql_real_connect() failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return false;
        }
        
        // Prepare the query
        const char* query = "SELECT COUNT(*) FROM users WHERE username = ?";
        stmt = mysql_stmt_init(conn);
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            std::cerr << "mysql_stmt_prepare() failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        // Bind parameters
        memset(bind, 0, sizeof(bind));
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)username.c_str();
        bind[0].buffer_length = username.length();
        
        if (mysql_stmt_bind_param(stmt, bind)) {
            std::cerr << "mysql_stmt_bind_param() failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        // Execute the query
        if (mysql_stmt_execute(stmt)) {
            std::cerr << "mysql_stmt_execute() failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        // Bind result
        memset(result, 0, sizeof(result));
        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = &count;
        
        if (mysql_stmt_bind_result(stmt, result)) {
            std::cerr << "mysql_stmt_bind_result() failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        // Fetch result
        if (mysql_stmt_fetch(stmt) == 0) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return count > 0;
        }
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
        
    } catch (...) {
        if (stmt) mysql_stmt_close(stmt);
        if (conn) mysql_close(conn);
        return false;
    }
}

int main() {
    std::string host = "localhost";
    std::string user = "root";
    std::string password = "password";
    std::string database = "testdb";
    
    // Test cases
    std::cout << "Test 1: " << checkUsernameExists("john_doe", host, user, password, database) << std::endl;
    std::cout << "Test 2: " << checkUsernameExists("jane_smith", host, user, password, database) << std::endl;
    std::cout << "Test 3: " << checkUsernameExists("admin", host, user, password, database) << std::endl;
    std::cout << "Test 4: " << checkUsernameExists("test_user", host, user, password, database) << std::endl;
    std::cout << "Test 5: " << checkUsernameExists("nonexistent", host, user, password, database) << std::endl;
    
    return 0;
}
