
#include <iostream>
#include <string>
#include <cstdlib>
#include <mysql/mysql.h>

class Lse139 {
public:
    static bool isModerator(const std::string& username) {
        const char* dbUrl = std::getenv("DB_URL");
        const char* dbUser = std::getenv("DB_USER");
        const char* dbPassword = std::getenv("DB_PASSWORD");
        const char* dbName = std::getenv("DB_NAME");
        
        if (!dbUrl || !dbUser || !dbPassword) {
            std::cerr << "Database credentials not found in environment variables" << std::endl;
            return false;
        }
        
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "mysql_init() failed" << std::endl;
            return false;
        }
        
        if (!mysql_real_connect(conn, dbUrl, dbUser, dbPassword, dbName, 0, nullptr, 0)) {
            std::cerr << "Connection failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return false;
        }
        
        std::string query = "SELECT is_moderator FROM users WHERE username = ?";
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        
        if (!stmt || mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
            std::cerr << "Statement preparation failed: " << mysql_error(conn) << std::endl;
            mysql_close(conn);
            return false;
        }
        
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)username.c_str();
        bind[0].buffer_length = username.length();
        
        mysql_stmt_bind_param(stmt, bind);
        mysql_stmt_execute(stmt);
        
        bool isMod = false;
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));
        
        result[0].buffer_type = MYSQL_TYPE_TINY;
        result[0].buffer = &isMod;
        
        mysql_stmt_bind_result(stmt, result);
        
        if (mysql_stmt_fetch(stmt) == 0) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return isMod;
        }
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return false;
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse139::isModerator("admin") << std::endl;
    std::cout << "Test 2: " << Lse139::isModerator("moderator1") << std::endl;
    std::cout << "Test 3: " << Lse139::isModerator("user123") << std::endl;
    std::cout << "Test 4: " << Lse139::isModerator("guest") << std::endl;
    std::cout << "Test 5: " << Lse139::isModerator("nonexistent") << std::endl;
    
    return 0;
}
