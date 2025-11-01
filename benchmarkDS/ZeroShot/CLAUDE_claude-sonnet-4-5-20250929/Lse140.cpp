
#include <iostream>
#include <string>
#include <mysql/mysql.h>

class Lse140 {
public:
    // Method to check if username exists in database
    static bool checkUsernameExists(const std::string& dbHost, const std::string& dbUser, 
                                   const std::string& dbPassword, const std::string& dbName,
                                   const std::string& username) {
        MYSQL* conn = nullptr;
        MYSQL_STMT* stmt = nullptr;
        MYSQL_BIND bind[1];
        MYSQL_BIND result[1];
        int count = 0;
        
        try {
            // Initialize MySQL connection
            conn = mysql_init(nullptr);
            if (!conn) {
                std::cerr << "MySQL initialization failed" << std::endl;
                return false;
            }
            
            // Connect to database
            if (!mysql_real_connect(conn, dbHost.c_str(), dbUser.c_str(), 
                                   dbPassword.c_str(), dbName.c_str(), 0, nullptr, 0)) {
                std::cerr << "Connection failed: " << mysql_error(conn) << std::endl;
                mysql_close(conn);
                return false;
            }
            
            // Prepare parameterized query
            const char* query = "SELECT COUNT(*) FROM users WHERE username = ?";
            stmt = mysql_stmt_init(conn);
            
            if (!stmt || mysql_stmt_prepare(stmt, query, strlen(query))) {
                std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
                if (stmt) mysql_stmt_close(stmt);
                mysql_close(conn);
                return false;
            }
            
            // Bind parameters
            memset(bind, 0, sizeof(bind));
            bind[0].buffer_type = MYSQL_TYPE_STRING;
            bind[0].buffer = (char*)username.c_str();
            bind[0].buffer_length = username.length();
            
            if (mysql_stmt_bind_param(stmt, bind)) {
                std::cerr << "Binding parameters failed: " << mysql_stmt_error(stmt) << std::endl;
                mysql_stmt_close(stmt);
                mysql_close(conn);
                return false;
            }
            
            // Execute query
            if (mysql_stmt_execute(stmt)) {
                std::cerr << "Query execution failed: " << mysql_stmt_error(stmt) << std::endl;
                mysql_stmt_close(stmt);
                mysql_close(conn);
                return false;
            }
            
            // Bind result
            memset(result, 0, sizeof(result));
            result[0].buffer_type = MYSQL_TYPE_LONG;
            result[0].buffer = (char*)&count;
            
            if (mysql_stmt_bind_result(stmt, result)) {
                std::cerr << "Binding result failed: " << mysql_stmt_error(stmt) << std::endl;
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
};

int main() {
    // Database connection parameters (adjust as needed)
    std::string dbHost = "localhost";
    std::string dbUser = "root";
    std::string dbPassword = "password";
    std::string dbName = "testdb";
    
    // Test cases
    std::cout << "Test Case 1: " << Lse140::checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "admin") << std::endl;
    std::cout << "Test Case 2: " << Lse140::checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "user123") << std::endl;
    std::cout << "Test Case 3: " << Lse140::checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "testuser") << std::endl;
    std::cout << "Test Case 4: " << Lse140::checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "john_doe") << std::endl;
    std::cout << "Test Case 5: " << Lse140::checkUsernameExists(dbHost, dbUser, dbPassword, dbName, "nonexistent") << std::endl;
    
    return 0;
}
