
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <regex>
#include <algorithm>

class Lse149 {
private:
    static const std::string DB_NAME;
    
public:
    // Initialize database
    static bool initializeDatabase() {
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS messages ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "username TEXT NOT NULL,"
                         "message TEXT NOT NULL,"
                         "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP);";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_close(db);
        return true;
    }
    
    // Sanitize input
    static std::string sanitizeInput(const std::string& input) {
        if (input.empty()) return "";
        
        std::string sanitized = input;
        
        // Remove dangerous characters
        sanitized = std::regex_replace(sanitized, std::regex("[<>\\"'&]"), "");
        
        // Remove script tags
        sanitized = std::regex_replace(sanitized, std::regex("(?i)(script|javascript|onerror|onload)"), "");
        
        // Trim whitespace
        sanitized.erase(sanitized.begin(), std::find_if(sanitized.begin(), sanitized.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        sanitized.erase(std::find_if(sanitized.rbegin(), sanitized.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), sanitized.end());
        
        return sanitized;
    }
    
    // Insert user message in database
    static bool insertUserMessageInDb(const std::string& username, const std::string& message) {
        if (username.empty() || message.empty()) {
            return false;
        }
        
        std::string sanitizedUsername = sanitizeInput(username);
        std::string sanitizedMessage = sanitizeInput(message);
        
        if (sanitizedUsername.empty() || sanitizedMessage.empty()) {
            return false;
        }
        
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, sanitizedUsername.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, sanitizedMessage.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return rc == SQLITE_DONE;
    }
    
    // Get all messages
    static std::vector<std::map<std::string, std::string>> getAllMessages() {
        std::vector<std::map<std::string, std::string>> messages;
        
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return messages;
        }
        
        const char* sql = "SELECT username, message, timestamp FROM messages ORDER BY timestamp DESC;";
        sqlite3_stmt* stmt;
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return messages;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<std::string, std::string> message;
            message["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            message["message"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            message["timestamp"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            messages.push_back(message);
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return messages;
    }
    
    // Handle POST route
    static std::string handlePostRoute(const std::string& username, const std::string& message) {
        if (username.empty() || message.empty()) {
            return "redirect:/error";
        }
        
        insertUserMessageInDb(username, message);
        return "redirect:/";
    }
    
    // Handle root route
    static std::string handleRootRoute() {
        auto messages = getAllMessages();
        std::string html = "<html><body><h1>Messages</h1>";
        
        for (const auto& msg : messages) {
            html += "<div><strong>" + msg.at("username") + "</strong>: " + 
                   msg.at("message") + " <em>(" + msg.at("timestamp") + ")</em></div><br>";
        }
        
        html += "</body></html>";
        return html;
    }
};

const std::string Lse149::DB_NAME = "messages.db";

int main() {
    // Initialize database
    Lse149::initializeDatabase();
    
    // Test case 1: Valid message
    std::cout << "Test 1: Valid message" << std::endl;
    std::cout << Lse149::handlePostRoute("Alice", "Hello World!") << std::endl;
    std::cout << Lse149::handleRootRoute() << std::endl << std::endl;
    
    // Test case 2: Another valid message
    std::cout << "Test 2: Another valid message" << std::endl;
    std::cout << Lse149::handlePostRoute("Bob", "Testing the system") << std::endl << std::endl;
    
    // Test case 3: Empty username
    std::cout << "Test 3: Empty username" << std::endl;
    std::cout << Lse149::handlePostRoute("", "This should fail") << std::endl << std::endl;
    
    // Test case 4: Empty message
    std::cout << "Test 4: Empty message" << std::endl;
    std::cout << Lse149::handlePostRoute("Charlie", "") << std::endl << std::endl;
    
    // Test case 5: XSS attempt
    std::cout << "Test 5: XSS attempt" << std::endl;
    std::cout << Lse149::handlePostRoute("Dave", "<script>alert('XSS')</script>Safe message") << std::endl;
    std::cout << Lse149::handleRootRoute() << std::endl;
    
    return 0;
}
