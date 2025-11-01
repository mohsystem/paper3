
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <sqlite3.h>

const int MAX_USERNAME_LENGTH = 50;
const int MAX_MESSAGE_LENGTH = 500;
const char* DB_PATH = "messages.db";

// HTML encode to prevent XSS attacks
std::string htmlEncode(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 1.2);
    
    for (char c : input) {
        switch (c) {
            case '&': output += "&amp;"; break;
            case '<': output += "&lt;"; break;
            case '>': output += "&gt;"; break;
            case '"': output += "&quot;"; break;
            case '\\'': output += "&#x27;"; break;
            case '/': output += "&#x2F;"; break;
            default: output += c; break;
        }
    }
    return output;
}

// Trim whitespace from string
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \\t\\n\\r");
    if (start == std::string::npos) return "";
    size_t end = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(start, end - start + 1);
}

// Initialize database with proper schema
bool initDatabase() {
    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH, &db);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Database open failed" << std::endl;
        if (db) sqlite3_close(db);
        return false;
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS messages ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL, "
        "message TEXT NOT NULL, "
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)";
    
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Database initialization failed" << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_close(db);
    return true;
}

// Secure database insertion using parameterized queries to prevent SQL injection
bool insertUserMessageInDb(const std::string& username, const std::string& message) {
    // Validate inputs before database operation
    if (username.empty() || message.empty()) {
        return false;
    }
    
    std::string cleanUsername = trim(username);
    std::string cleanMessage = trim(message);
    
    if (cleanUsername.empty() || cleanMessage.empty()) {
        return false;
    }
    
    if (cleanUsername.length() > MAX_USERNAME_LENGTH || 
        cleanMessage.length() > MAX_MESSAGE_LENGTH) {
        return false;
    }
    
    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH, &db);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Database open failed" << std::endl;
        if (db) sqlite3_close(db);
        return false;
    }
    
    // Use parameterized query to prevent SQL injection
    const char* sql = "INSERT INTO messages (username, message) VALUES (?, ?)";
    sqlite3_stmt* stmt = nullptr;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Statement preparation failed" << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, cleanUsername.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, cleanMessage.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

// Retrieve messages from database with limit to prevent resource exhaustion
std::vector<std::map<std::string, std::string>> getMessages() {
    std::vector<std::map<std::string, std::string>> messages;
    
    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH, &db);
    
    if (rc != SQLITE_OK) {
        std::cerr << "Database open failed" << std::endl;
        if (db) sqlite3_close(db);
        return messages;
    }
    
    const char* sql = "SELECT username, message, created_at FROM messages "
                     "ORDER BY created_at DESC LIMIT 100";
    sqlite3_stmt* stmt = nullptr;
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Statement preparation failed" << std::endl;
        sqlite3_close(db);
        return messages;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::map<std::string, std::string> msg;
        
        const unsigned char* username = sqlite3_column_text(stmt, 0);
        const unsigned char* message = sqlite3_column_text(stmt, 1);
        const unsigned char* created_at = sqlite3_column_text(stmt, 2);
        
        if (username) msg["username"] = reinterpret_cast<const char*>(username);
        if (message) msg["message"] = reinterpret_cast<const char*>(message);
        if (created_at) msg["created_at"] = reinterpret_cast<const char*>(created_at);
        
        messages.push_back(msg);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return messages;
}

// Generate HTML page with XSS protection
std::string generateHtmlPage() {
    std::vector<std::map<std::string, std::string>> messages = getMessages();
    
    std::ostringstream html;
    html << "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">";
    html << "<title>Messages</title></head><body>";
    html << "<h1>Post a Message</h1>";
    html << "<form method=\\"POST\\" action=\\"/post\\">";
    html << "Username: <input type=\\"text\\" name=\\"username\\" maxlength=\\"50\\" required><br>";
    html << "Message: <textarea name=\\"message\\" maxlength=\\"500\\" required></textarea><br>";
    html << "<input type=\\"submit\\" value=\\"Post\\"></form>";
    html << "<h2>Messages:</h2><ul>";
    
    for (const auto& msg : messages) {
        auto it_user = msg.find("username");
        auto it_msg = msg.find("message");
        auto it_time = msg.find("created_at");
        
        if (it_user != msg.end() && it_msg != msg.end()) {
            html << "<li><strong>" << htmlEncode(it_user->second) << "</strong>: ";
            html << htmlEncode(it_msg->second);
            if (it_time != msg.end()) {
                html << " <em>(" << htmlEncode(it_time->second) << ")</em>";
            }
            html << "</li>";
        }
    }
    
    html << "</ul></body></html>";
    return html.str();
}

int main() {
    if (!initDatabase()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    
    // Test cases
    std::cout << "Test 1: Valid input: " << insertUserMessageInDb("Alice", "Hello World") << std::endl;
    std::cout << "Test 2: Valid input: " << insertUserMessageInDb("Bob", "Test message") << std::endl;
    std::cout << "Test 3: Empty username: " << insertUserMessageInDb("", "Message") << std::endl;
    std::cout << "Test 4: Empty message: " << insertUserMessageInDb("User", "") << std::endl;
    std::cout << "Test 5: Too long username: " << insertUserMessageInDb(std::string(100, 'a'), "msg") << std::endl;
    
    // Display messages
    std::string htmlPage = generateHtmlPage();
    std::cout << "\\nGenerated HTML Page:\\n" << htmlPage << std::endl;
    
    return 0;
}
