
#include <iostream>
#include <string>
#include <cstdlib>
#include <mysql/mysql.h>  // For MySQL, adjust for other databases

class Lse139 {
public:
    static bool isUserModerator(const std::string& username) {
        const char* dbHost = std::getenv("DB_HOST");
        const char* dbUser = std::getenv("DB_USER");
        const char* dbPassword = std::getenv("DB_PASSWORD");
        const char* dbName = std::getenv("DB_NAME");
        
        if (!dbHost || !dbUser || !dbPassword || !dbName) {
            std::cerr << "Database credentials not found in environment variables" << std::endl;
            return false;
        }
        
        MYSQL* conn = mysql_init(nullptr);
        
        if (!conn) {
            std::cerr << "MySQL initialization failed" << std::endl;
            return false;
        }
        
        if (!mysql_real_connect(conn, dbHost, dbUser, dbPassword, dbName, 0, nullptr, 0)) {
            std::cerr << "Database connection failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return false;
        }
        
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        const char* query = "SELECT is_moderator FROM users WHERE username = ?";
        
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            std::cerr << "Statement preparation failed: " << mysql_stmt_error(stmt) << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)username.c_str();
        bind[0].buffer_length = username.length();
        
        if (mysql_stmt_bind_param(stmt, bind)) {
            std::cerr << "Parameter binding failed" << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        if (mysql_stmt_execute(stmt)) {
            std::cerr << "Query execution failed" << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        bool isModerator = false;
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));
        
        result[0].buffer_type = MYSQL_TYPE_TINY;
        result[0].buffer = &isModerator;
        
        mysql_stmt_bind_result(stmt, result);
        
        if (mysql_stmt_fetch(stmt) == 0) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return isModerator;
        }
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse139::isUserModerator("admin_user") << std::endl;
    std::cout << "Test 2: " << Lse139::isUserModerator("moderator1") << std::endl;
    std::cout << "Test 3: " << Lse139::isUserModerator("regular_user") << std::endl;
    std::cout << "Test 4: " << Lse139::isUserModerator("test_mod") << std::endl;
    std::cout << "Test 5: " << Lse139::isUserModerator("guest_user") << std::endl;
    
    return 0;
}
