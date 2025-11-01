
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <algorithm>
#include <ctime>
#include <map>

class Lse150 {
private:
    sqlite3* db;
    
    // Sanitize input to prevent XSS and SQL injection
    std::string sanitizeInput(const std::string& input) {
        if (input.empty()) return "";
        
        std::string sanitized = input;
        
        // Remove dangerous characters
        sanitized.erase(
            std::remove_if(sanitized.begin(), sanitized.end(),
                [](char c) { return c == '<' || c == '>' || c == '"' || 
                            c == '\\'' || c == '&'; }),
            sanitized.end()
        );
        
        // Limit length
        if (sanitized.length() > 500) {
            sanitized = sanitized.substr(0, 500);
        }
        
        return sanitized;
    }
    
    // Get current timestamp
    std::string getCurrentTimestamp() {
        time_t now = time(0);
        char buffer[80];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return std::string(buffer);
    }
    
public:
    Lse150() : db(nullptr) {
        // Initialize database
        int rc = sqlite3_open("messages.db", &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        // Create table
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS messages ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT NOT NULL,"
            "message TEXT NOT NULL,"
            "timestamp TEXT NOT NULL);";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }
    
    ~Lse150() {
        if (db) {
            sqlite3_close(db);
        }
    }
    
    // Insert user message into database
    bool insert_user_message_in_db(const std::string& username, const std::string& message) {
        if (username.empty() || message.empty()) {
            return false;
        }
        
        // Sanitize inputs
        std::string cleanUsername = sanitizeInput(username);
        std::string cleanMessage = sanitizeInput(message);
        
        if (cleanUsername.empty() || cleanMessage.empty()) {
            return false;
        }
        
        // Use prepared statement to prevent SQL injection
        const char* sql = "INSERT INTO messages (username, message, timestamp) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, cleanUsername.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, cleanMessage.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, getCurrentTimestamp().c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        return rc == SQLITE_DONE;
    }
    
    // Post function - simulates handling POST request
    bool post(const std::map<std::string, std::string>& request) {
        auto usernameIt = request.find("username");
        auto messageIt = request.find("message");
        
        if (usernameIt == request.end() || messageIt == request.end()) {
            return false;
        }
        
        std::string username = usernameIt->second;
        std::string message = messageIt->second;
        
        if (!username.empty() && !message.empty()) {
            return insert_user_message_in_db(username, message);
        }
        
        return false;
    }
};

// Test cases
int main() {
    std::cout << "Running test cases:" << std::endl;
    
    Lse150 handler;
    
    // Test case 1: Valid input
    std::cout << "Test 1: Valid input - ";
    std::map<std::string, std::string> test1 = {{"username", "john_doe"}, {"message", "Hello World!"}};
    std::cout << (handler.post(test1) ? "Success" : "Failed") << std::endl;
    
    // Test case 2: Empty username
    std::cout << "Test 2: Empty username - ";
    std::map<std::string, std::string> test2 = {{"username", ""}, {"message", "Test message"}};
    std::cout << (handler.post(test2) ? "Success" : "Failed") << std::endl;
    
    // Test case 3: Empty message
    std::cout << "Test 3: Empty message - ";
    std::map<std::string, std::string> test3 = {{"username", "jane_doe"}, {"message", ""}};
    std::cout << (handler.post(test3) ? "Success" : "Failed") << std::endl;
    
    // Test case 4: XSS attempt
    std::cout << "Test 4: XSS attempt - ";
    std::map<std::string, std::string> test4 = {{"username", "attacker"}, {"message", "<script>alert('XSS')</script>"}};
    std::cout << (handler.post(test4) ? "Success" : "Failed") << std::endl;
    
    // Test case 5: SQL injection attempt
    std::cout << "Test 5: SQL injection attempt - ";
    std::map<std::string, std::string> test5 = {{"username", "hacker"}, {"message", "'; DROP TABLE messages; --"}};
    std::cout << (handler.post(test5) ? "Success" : "Failed") << std::endl;
    
    return 0;
}
