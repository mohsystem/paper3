
#include <iostream>
#include <mysql/mysql.h>
#include <string>
#include <cstring>

class Lse140 {
private:
    static constexpr const char* DB_HOST = "localhost";
    static constexpr const char* DB_USER = "root";
    static constexpr const char* DB_PASSWORD = "password";
    static constexpr const char* DB_NAME = "testdb";

public:
    static bool checkUsernameExists(const std::string& username) {
        if (username.empty()) {
            return false;
        }
        
        MYSQL* conn = mysql_init(nullptr);
        if (!conn) {
            std::cerr << "MySQL initialization failed" << std::endl;
            return false;
        }
        
        if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, 
                                DB_NAME, 0, nullptr, 0)) {
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
        
        const char* query = "SELECT COUNT(*) FROM users WHERE username = ?";
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
            std::cerr << "Statement execution failed" << std::endl;
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        int count = 0;
        MYSQL_BIND result[1];
        memset(result, 0, sizeof(result));
        result[0].buffer_type = MYSQL_TYPE_LONG;
        result[0].buffer = (char*)&count;
        
        mysql_stmt_bind_result(stmt, result);
        mysql_stmt_fetch(stmt);
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        
        return count > 0;
    }
};

int main() {
    std::string testUsers[] = {"admin", "john_doe", "'; DROP TABLE users; --", 
                               "user123", "nonexistent"};
    
    std::cout << "Testing username existence:" << std::endl;
    for (const auto& user : testUsers) {
        bool exists = Lse140::checkUsernameExists(user);
        std::cout << "Username '" << user << "' exists: " 
                  << (exists ? "true" : "false") << std::endl;
    }
    
    return 0;
}
