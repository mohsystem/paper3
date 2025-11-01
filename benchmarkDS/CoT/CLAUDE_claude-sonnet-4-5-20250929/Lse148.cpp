
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <ctime>
#include <algorithm>

class Lse148 {
private:
    static const int MAX_USERNAME_LENGTH = 50;
    static const int MAX_MESSAGE_LENGTH = 500;
    static constexpr const char* DB_NAME = "messages.db";
    
    static std::string validateAndSanitizeInput(const std::string& input, int maxLength) {
        if (input.empty()) {
            return "";
        }
        
        std::string sanitized = input;
        
        // Trim whitespace
        sanitized.erase(0, sanitized.find_first_not_of(" \\t\\n\\r"));
        sanitized.erase(sanitized.find_last_not_of(" \\t\\n\\r") + 1);
        
        if (sanitized.empty()) {
            return "";
        }
        
        // Truncate if too long
        if (sanitized.length() > static_cast<size_t>(maxLength)) {
            sanitized = sanitized.substr(0, maxLength);
        }
        
        // Remove dangerous characters
        std::string dangerous = "<>\\"'&";
        for (char c : dangerous) {
            sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), c), sanitized.end());
        }
        
        return sanitized;
    }
    
    static std::string htmlEncode(const std::string& input) {
        std::string output;
        for (char c : input) {
            switch (c) {
                case '&': output += "&amp;"; break;
                case '<': output += "&lt;"; break;
                case '>': output += "&gt;"; break;
                case '"': output += "&quot;"; break;
                case '\\'': output += "&#x27;"; break;
                default: output += c; break;
            }
        }
        return output;
    }
    
    static std::string getCurrentTimestamp() {
        time_t now = time(nullptr);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return std::string(buf);
    }
    
public:
    static bool insert_user_message_in_db(const std::string& username, const std::string& message) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        const char* sql = "INSERT INTO messages (username, message, timestamp) VALUES (?, ?, ?)";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Prepare error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, getCurrentTimestamp().c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return rc == SQLITE_DONE;
    }
    
    static bool post(const std::map<std::string, std::string>& request) {
        try {
            auto usernameIt = request.find("username");
            auto messageIt = request.find("message");
            
            if (usernameIt == request.end() || messageIt == request.end()) {
                return false;
            }
            
            std::string username = validateAndSanitizeInput(usernameIt->second, MAX_USERNAME_LENGTH);
            std::string message = validateAndSanitizeInput(messageIt->second, MAX_MESSAGE_LENGTH);
            
            if (username.empty() || message.empty()) {
                std::cerr << "Invalid input: username or message is empty" << std::endl;
                return false;
            }
            
            if (insert_user_message_in_db(username, message)) {
                std::cout << "Redirecting to: /main" << std::endl;
                return true;
            }
            
            return false;
            
        } catch (const std::exception& e) {
            std::cerr << "Error in post: " << e.what() << std::endl;
            return false;
        }
    }
    
    static std::vector<std::string> displayMessages() {
        std::vector<std::string> messages;
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return messages;
        }
        
        const char* sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC LIMIT 100";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Prepare error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return messages;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string username = htmlEncode(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            std::string message = htmlEncode(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            std::string timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            
            messages.push_back("[" + timestamp + "] " + username + ": " + message);
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return messages;
    }
    
    static void initDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Database init error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "username TEXT NOT NULL,"
                         "message TEXT NOT NULL,"
                         "timestamp DATETIME NOT NULL)";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
};

int main() {
    Lse148::initDatabase();
    
    std::cout << "=== Test Cases ===\\n" << std::endl;
    
    // Test 1: Valid message
    std::map<std::string, std::string> request1 = {{"username", "Alice"}, {"message", "Hello World!"}};
    std::cout << "Test 1 - Valid message: " << std::boolalpha << Lse148::post(request1) << std::endl;
    
    // Test 2: Another valid message
    std::map<std::string, std::string> request2 = {{"username", "Bob"}, {"message", "This is a test message"}};
    std::cout << "Test 2 - Valid message: " << std::boolalpha << Lse148::post(request2) << std::endl;
    
    // Test 3: Empty username (should fail)
    std::map<std::string, std::string> request3 = {{"username", ""}, {"message", "Message without user"}};
    std::cout << "Test 3 - Empty username: " << std::boolalpha << Lse148::post(request3) << std::endl;
    
    // Test 4: XSS attempt (should be sanitized)
    std::map<std::string, std::string> request4 = {{"username", "Hacker<script>"}, {"message", "Alert<script>alert('XSS')</script>"}};
    std::cout << "Test 4 - XSS attempt: " << std::boolalpha << Lse148::post(request4) << std::endl;
    
    // Test 5: Very long message (should be truncated)
    std::map<std::string, std::string> request5 = {{"username", "Charlie"}, {"message", std::string(1000, 'A')}};
    std::cout << "Test 5 - Long message: " << std::boolalpha << Lse148::post(request5) << std::endl;
    
    // Display all messages
    std::cout << "\\n=== Displaying Messages ===" << std::endl;
    std::vector<std::string> messages = Lse148::displayMessages();
    for (const auto& msg : messages) {
        std::cout << msg << std::endl;
    }
    
    return 0;
}
