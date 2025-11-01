
#include <iostream>
#include <string>
#include <map>
#include <sqlite3.h>
#include <algorithm>
#include <ctime>

class Lse150 {
private:
    static const int MAX_USERNAME_LENGTH = 50;
    static const int MAX_MESSAGE_LENGTH = 500;
    
    static std::string sanitizeInput(const std::string& input) {
        std::string result = input;
        std::string unsafe = "<>\\"'&";
        
        result.erase(std::remove_if(result.begin(), result.end(),
            [&unsafe](char c) { return unsafe.find(c) != std::string::npos; }),
            result.end());
        
        // Trim whitespace
        result.erase(0, result.find_first_not_of(" \\t\\n\\r"));
        result.erase(result.find_last_not_of(" \\t\\n\\r") + 1);
        
        return result;
    }
    
    static bool validateInput(const std::string& username, const std::string& message) {
        return !username.empty() && username.length() <= MAX_USERNAME_LENGTH &&
               !message.empty() && message.length() <= MAX_MESSAGE_LENGTH;
    }

public:
    static bool insert_user_message_in_db(const std::string& username, const std::string& message) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        bool success = false;
        
        if (sqlite3_open("messages.db", &db) != SQLITE_OK) {
            std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        const char* sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, ?)";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            std::string sanitized_username = sanitizeInput(username);
            std::string sanitized_message = sanitizeInput(message);
            time_t now = time(nullptr);
            
            sqlite3_bind_text(stmt, 1, sanitized_username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, sanitized_message.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_int64(stmt, 3, now);
            
            if (sqlite3_step(stmt) == SQLITE_DONE) {
                success = true;
            }
            
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
        return success;
    }
    
    static std::string post(const std::map<std::string, std::string>& request) {
        auto username_it = request.find("username");
        auto message_it = request.find("message");
        
        if (username_it == request.end() || message_it == request.end()) {
            return "error.html?msg=Missing parameters";
        }
        
        std::string username = username_it->second;
        std::string message = message_it->second;
        
        if (!validateInput(username, message)) {
            return "error.html?msg=Invalid input";
        }
        
        if (insert_user_message_in_db(username, message)) {
            return "redirect:/index.html";
        } else {
            return "error.html?msg=Failed to save message";
        }
    }
    
    static std::string getMainPageForm() {
        return R"(<!DOCTYPE html>\n<html>\n<head>\n<meta charset="UTF-8">\n<title>Message Board</title>\n</head>\n<body>\n<h1>Submit a Message</h1>\n<form method="POST" action="/post">\n  <label>Username:</label><br>\n  <input type="text" name="username" maxlength="50" required><br><br>\n  <label>Message:</label><br>\n  <textarea name="message" maxlength="500" required></textarea><br><br>\n  <input type="submit" value="Submit">\n</form>\n</body>\n</html>)";
    }
};

int main() {
    std::cout << "Test Case 1: Valid input" << std::endl;
    std::map<std::string, std::string> req1 = {{"username", "John"}, {"message", "Hello World"}};
    std::cout << Lse150::post(req1) << std::endl;
    
    std::cout << "\\nTest Case 2: Empty username" << std::endl;
    std::map<std::string, std::string> req2 = {{"username", ""}, {"message", "Test message"}};
    std::cout << Lse150::post(req2) << std::endl;
    
    std::cout << "\\nTest Case 3: Missing message" << std::endl;
    std::map<std::string, std::string> req3 = {{"username", "Alice"}};
    std::cout << Lse150::post(req3) << std::endl;
    
    std::cout << "\\nTest Case 4: XSS attempt" << std::endl;
    std::map<std::string, std::string> req4 = {{"username", "<script>alert('xss')</script>"}, {"message", "Normal message"}};
    std::cout << Lse150::post(req4) << std::endl;
    
    std::cout << "\\nTest Case 5: Long input" << std::endl;
    std::map<std::string, std::string> req5 = {{"username", std::string(60, 'A')}, {"message", "Test"}};
    std::cout << Lse150::post(req5) << std::endl;
    
    return 0;
}
