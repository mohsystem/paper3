
#include <iostream>
#include <string>
#include <mysql/mysql.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

using namespace web;
using namespace web::http;
using namespace web::http::experimental::listener;

class Lse138 {
private:
    const char* DB_HOST = "localhost";
    const char* DB_USER = "root";
    const char* DB_PASSWORD = "password";
    const char* DB_NAME = "webapp";
    
public:
    bool checkIfModerator(const std::string& username) {
        if (username.empty()) {
            return false;
        }
        
        MYSQL* conn = mysql_init(NULL);
        if (conn == NULL) {
            return false;
        }
        
        if (mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASSWORD, 
                              DB_NAME, 0, NULL, 0) == NULL) {
            mysql_close(conn);
            return false;
        }
        
        MYSQL_STMT* stmt = mysql_stmt_init(conn);
        std::string query = "SELECT is_moderator FROM users WHERE username = ?";
        
        if (mysql_stmt_prepare(stmt, query.c_str(), query.length()) != 0) {
            mysql_stmt_close(stmt);
            mysql_close(conn);
            return false;
        }
        
        MYSQL_BIND bind[1];
        memset(bind, 0, sizeof(bind));
        
        unsigned long username_length = username.length();
        bind[0].buffer_type = MYSQL_TYPE_STRING;
        bind[0].buffer = (char*)username.c_str();
        bind[0].buffer_length = username_length;
        bind[0].length = &username_length;
        
        mysql_stmt_bind_param(stmt, bind);
        mysql_stmt_execute(stmt);
        
        bool is_moderator = false;
        MYSQL_BIND result_bind[1];
        memset(result_bind, 0, sizeof(result_bind));
        
        result_bind[0].buffer_type = MYSQL_TYPE_TINY;
        result_bind[0].buffer = (char*)&is_moderator;
        
        mysql_stmt_bind_result(stmt, result_bind);
        
        if (mysql_stmt_fetch(stmt) == 0) {
            // Result fetched successfully
        } else {
            is_moderator = false;
        }
        
        mysql_stmt_close(stmt);
        mysql_close(conn);
        
        return is_moderator;
    }
    
    void handleRequest(http_request request) {
        auto query = uri::split_query(request.relative_uri().query());
        std::string username;
        
        if (query.find(U("username")) != query.end()) {
            username = utility::conversions::to_utf8string(query[U("username")]);
        }
        
        bool isModerator = checkIfModerator(username);
        
        json::value response;
        response[U("isModerator")] = json::value::boolean(isModerator);
        
        request.reply(status_codes::OK, response);
    }
};

int main() {
    // Test cases
    Lse138 app;
    std::string testUsers[] = {"admin", "moderator1", "user1", "testmod", "normaluser"};
    
    std::cout << "Test Cases:" << std::endl;
    for (const auto& user : testUsers) {
        bool result = app.checkIfModerator(user);
        std::cout << "User: " << user << " - Is Moderator: " << result << std::endl;
    }
    
    // Web server setup
    http_listener listener(U("http://localhost:8080/checkModerator"));
    
    listener.support(methods::GET, [&app](http_request request) {
        app.handleRequest(request);
    });
    
    try {
        listener.open().wait();
        std::cout << "Server running on http://localhost:8080/checkModerator" << std::endl;
        std::cout << "Press ENTER to exit." << std::endl;
        std::string line;
        std::getline(std::cin, line);
        listener.close().wait();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
