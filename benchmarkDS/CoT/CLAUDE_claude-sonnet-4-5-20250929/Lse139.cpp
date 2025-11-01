
#include <iostream>
#include <string>
#include <cstdlib>
#include <mysql/mysql.h>

class Lse139 {
public:
    static bool isUserModerator(const std::string& username) {
        if (username.empty()) {
            return false;
        }
        
        const char* dbHost = std::getenv("DB_HOST");
        const char* dbUser = std::getenv("DB_USER");
        const char* dbPassword = std::getenv("DB_PASSWORD");
        const char* dbName = std::getenv("DB_NAME");
        
        if (!dbHost || !dbUser || !dbPassword || !dbName) {
            std::cerr << "Database credentials not configured" << std::endl;
            return false;
        }
        
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "MySQL initialization failed" << std::endl;
            return false;
        }
        
        if (!mysql_real_connect(conn, dbHost, dbUser, dbPassword, dbName, 0, nullptr, 0)) {
            std::cerr << "Database connection failed" << std::endl;
            mysql_close(conn);
            return false;
        }
        
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        if (!stmt) {
            std::cerr << "Statement initialization failed" << std::endl;
            mysql_close(conn);
            return false;
        }
        
        const char* query = "SELECT is_moderator FROM users WHERE username = ? LIMIT 1";
        if (mysql_stmt_prepare(stmt, query, strlen(query))) {
            std::cerr << "Statement preparation failed" << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        
        unsigned long length = username.length();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)username.c_str();
        bind[0].buffer_length = username.length();
        bind[0].length = &length;
        
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
        result[0].buffer = (char*)&isModerator;
        result[0].is_null = 0;
        result[0].length = 0;
        
        mysql_stmt_bind_result(stmt, result);
        
        int fetchResult = mysql_stmt_fetch(stmt);
        bool returnValue = (fetchResult == 0) ? isModerator : false;
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        
        return returnValue;
    }
};

int main() {
    std::cout << "Test 1 - Valid moderator: " << Lse139::isUserModerator("admin") << std::endl;
    std::cout << "Test 2 - Valid non-moderator: " << Lse139::isUserModerator("user123") << std::endl;
    std::cout << "Test 3 - Non-existent user: " << Lse139::isUserModerator("nonexistent") << std::endl;
    std::cout << "Test 4 - Empty username: " << Lse139::isUserModerator("") << std::endl;
    std::cout << "Test 5 - Another test: " << Lse139::isUserModerator("testuser") << std::endl;
    
    return 0;
}
