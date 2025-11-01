
#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <map>
#include <sqlite3.h>
#include <algorithm>

class Lse148 {
private:
    sqlite3* db;
    
    // Sanitize input to prevent SQL injection and XSS
    std::string sanitizeInput(const std::string& input) {
        std::string sanitized = input;
        // Remove dangerous characters
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '<'), sanitized.end());
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '>'), sanitized.end());
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\\"'), sanitized.end());
        sanitized.erase(std::remove(sanitized.begin(), sanitized.end(), '\\''), sanitized.end());
        
        // Limit length
        if (sanitized.length() > 500) {
            sanitized = sanitized.substr(0, 500);
        }
        
        return sanitized;
    }
    
    // Escape HTML for display
    std::string escapeHtml(const std::string& text) {
        std::string escaped;
        for (char c : text) {
            switch(c) {
                case '&': escaped += "&amp;"; break;
                case '<': escaped += "&lt;"; break;
                case '>': escaped += "&gt;"; break;
                case '"': escaped += "&quot;"; break;
                case '\\'': escaped += "&#x27;"; break;
                default: escaped += c; break;
            }
        }
        return escaped;
    }
    
public:
    Lse148() {
        initDatabase();
    }
    
    ~Lse148() {
        if (db) {
            sqlite3_close(db);
        }
    }
    
    // Initialize database
    void initDatabase() {
        int rc = sqlite3_open("messages.db", &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
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
        }
    }
    
    // Insert user message in database
    void insert_user_message_in_db(const std::string& username, const std::string& message) {
        if (username.empty() || message.empty()) {
            return;
        }
        
        const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_TRANSIENT);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
            }
            
            sqlite3_finalize(stmt);
        }
    }
    
    // Get messages from database
    std::vector<std::map<std::string, std::string>> getMessages() {
        std::vector<std::map<std::string, std::string>> messages;
        
        const char* sql = "SELECT username, message, timestamp FROM messages "
                         "ORDER BY timestamp DESC LIMIT 50;";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                std::map<std::string, std::string> msg;
                msg["username"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                msg["message"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                msg["timestamp"] = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                messages.push_back(msg);
            }
            sqlite3_finalize(stmt);
        }
        
        return messages;
    }
    
    // POST function - simulates handling a POST request
    void post(const std::string& username, const std::string& message) {
        // Get username and message from request (parameters)
        std::string sanitizedUsername = sanitizeInput(username);
        std::string sanitizedMessage = sanitizeInput(message);
        
        // Validate input
        if (!sanitizedUsername.empty() && !sanitizedMessage.empty()) {
            // Insert message in database
            insert_user_message_in_db(sanitizedUsername, sanitizedMessage);
        }
        
        // In a real web application, this would redirect to main page
        std::cout << "Message posted successfully. Redirecting to main page..." << std::endl;
    }
    
    // Display messages
    void displayMessages() {
        std::vector<std::map<std::string, std::string>> messages = getMessages();
        
        std::cout << "\\n========== MESSAGE BOARD ==========" << std::endl;
        std::cout << "Total Messages: " << messages.size() << std::endl;
        std::cout << "===================================" << std::endl;
        
        for (const auto& msg : messages) {
            std::cout << "\\n[" << msg.at("username") << "] " 
                     << "(" << msg.at("timestamp") << ")" << std::endl;
            std::cout << msg.at("message") << std::endl;
            std::cout << "-----------------------------------" << std::endl;
        }
    }
};

int main() {
    Lse148 app;
    
    std::cout << "Running test cases...\\n" << std::endl;
    
    // Test case 1: Normal message
    std::cout << "Test 1: Posting normal message" << std::endl;
    app.post("Alice", "Hello, this is my first message!");
    
    // Test case 2: Another user
    std::cout << "\\nTest 2: Posting from another user" << std::endl;
    app.post("Bob", "Great to be here!");
    
    // Test case 3: Message with special characters
    std::cout << "\\nTest 3: Testing special characters handling" << std::endl;
    app.post("Charlie", "Testing <script>alert('xss')</script> security");
    
    // Test case 4: Long message
    std::cout << "\\nTest 4: Testing long message" << std::endl;
    app.post("David", "This is a very long message to test the length limit functionality of the system.");
    
    // Test case 5: Empty input
    std::cout << "\\nTest 5: Testing empty input" << std::endl;
    app.post("", "");
    
    // Display all messages
    std::cout << "\\n\\nDisplaying all messages:" << std::endl;
    app.displayMessages();
    
    return 0;
}
