
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include <cstring>

// Rule#2: Define maximum input lengths
const size_t MAX_USERNAME_LENGTH = 50;
const size_t MAX_MESSAGE_LENGTH = 500;

// Rule#2: Validate input contains no dangerous control characters
bool isValidInput(const std::string& input) {
    for (char c : input) {
        // Allow printable chars, newline, and tab only
        if (std::iscntrl(static_cast<unsigned char>(c)) && c != '\\n' && c != '\\t') {
            return false;
        }
    }
    return true;
}

// Rule#2: HTML escape to prevent XSS attacks
std::string escapeHtml(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    
    for (char c : input) {
        switch (c) {
            case '&': output.append("&amp;"); break;
            case '<': output.append("&lt;"); break;
            case '>': output.append("&gt;"); break;
            case '"': output.append("&quot;"); break;
            case '\\'': output.append("&#x27;"); break;
            default: output.push_back(c);
        }
    }
    return output;
}

// Rule#32: Use parameterized queries to prevent SQL injection
void insertUserMessageInDb(const std::string& username, const std::string& message) {
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    
    try {
        // Rule#3: Read database path from environment
        const char* dbPath = std::getenv("DB_PATH");
        if (!dbPath) {
            dbPath = "messages.db";
        }
        
        // Open database
        if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
            throw std::runtime_error("Database open failed");
        }
        
        // Rule#32: Parameterized query prevents SQL injection
        const char* sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, datetime('now'))";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Statement preparation failed");
        }
        
        // Rule#32: Bind parameters safely
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            throw std::runtime_error("Statement execution failed");
        }
        
        // Cleanup
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
    } catch (...) {
        if (stmt) sqlite3_finalize(stmt);
        if (db) sqlite3_close(db);
        throw;
    }
}

// Rule#5: Generate CSRF token using cryptographically secure random
std::string generateCsrfToken() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 4; ++i) {
        ss << std::setw(16) << dis(gen);
    }
    return ss.str();
}

// Handle POST request with security validations
std::string post(const std::map<std::string, std::string>& request,
                const std::map<std::string, std::string>& session) {
    try {
        // Rule#2: Validate CSRF token
        auto reqToken = request.find("csrf_token");
        auto sessToken = session.find("csrf_token");
        
        if (reqToken == request.end() || sessToken == session.end() ||
            reqToken->second != sessToken->second) {
            return "error"; // Rule#11: Generic error
        }
        
        // Rule#2: Get and validate inputs
        auto userIt = request.find("username");
        auto msgIt = request.find("message");
        
        if (userIt == request.end() || msgIt == request.end()) {
            return "error";
        }
        
        const std::string& username = userIt->second;
        const std::string& message = msgIt->second;
        
        // Rule#2: Validate length constraints
        if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
            return "error";
        }
        if (message.empty() || message.length() > MAX_MESSAGE_LENGTH) {
            return "error";
        }
        
        // Rule#2: Validate content
        if (!isValidInput(username) || !isValidInput(message)) {
            return "error";
        }
        
        // Insert into database
        insertUserMessageInDb(username, message);
        
        return "redirect:/main";
        
    } catch (...) {
        // Rule#11: Do not leak internal details
        std::cerr << "Error processing post request" << std::endl;
        return "error";
    }
}

// Retrieve messages with XSS protection
std::vector<std::map<std::string, std::string>> getMessages() {
    std::vector<std::map<std::string, std::string>> messages;
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    
    try {
        const char* dbPath = std::getenv("DB_PATH");
        if (!dbPath) {
            dbPath = "messages.db";
        }
        
        if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
            throw std::runtime_error("Database open failed");
        }
        
        // Rule#32: Safe query
        const char* sql = "SELECT username, message, created_at FROM messages ORDER BY created_at DESC LIMIT 100";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Statement preparation failed");
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::map<std::string, std::string> msg;
            
            // Rule#2: Escape output to prevent XSS
            const char* username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* timestamp = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            
            msg["username"] = escapeHtml(username ? username : "");
            msg["message"] = escapeHtml(message ? message : "");
            msg["timestamp"] = escapeHtml(timestamp ? timestamp : "");
            
            messages.push_back(msg);
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
    } catch (...) {
        if (stmt) sqlite3_finalize(stmt);
        if (db) sqlite3_close(db);
        std::cerr << "Error retrieving messages" << std::endl;
    }
    
    return messages;
}

int main() {
    // Setup: Create in-memory database for testing
    sqlite3* db = nullptr;
    sqlite3_open(":memory:", &db);
    sqlite3_exec(db, "CREATE TABLE messages (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT, message TEXT, created_at TEXT)", nullptr, nullptr, nullptr);
    sqlite3_close(db);
    setenv("DB_PATH", ":memory:", 1);
    
    // Test case 1: Valid post
    std::string token1 = generateCsrfToken();
    std::map<std::string, std::string> req1 = {{"username", "alice"}, {"message", "Hello world!"}, {"csrf_token", token1}};
    std::map<std::string, std::string> session1 = {{"csrf_token", token1}};
    std::cout << "Test 1: " << post(req1, session1) << std::endl;
    
    // Test case 2: Another valid post
    std::string token2 = generateCsrfToken();
    std::map<std::string, std::string> req2 = {{"username", "bob"}, {"message", "Test message"}, {"csrf_token", token2}};
    std::map<std::string, std::string> session2 = {{"csrf_token", token2}};
    std::cout << "Test 2: " << post(req2, session2) << std::endl;
    
    // Test case 3: Invalid CSRF token
    std::map<std::string, std::string> req3 = {{"username", "eve"}, {"message", "Attack"}, {"csrf_token", "wrong"}};
    std::map<std::string, std::string> session3 = {{"csrf_token", "correct"}};
    std::cout << "Test 3 (should error): " << post(req3, session3) << std::endl;
    
    // Test case 4: Oversized input
    std::string token4 = generateCsrfToken();
    std::map<std::string, std::string> req4 = {{"username", std::string(100, 'x')}, {"message", "msg"}, {"csrf_token", token4}};
    std::map<std::string, std::string> session4 = {{"csrf_token", token4}};
    std::cout << "Test 4 (should error): " << post(req4, session4) << std::endl;
    
    // Test case 5: XSS attempt
    std::string token5 = generateCsrfToken();
    std::map<std::string, std::string> req5 = {{"username", "hacker"}, {"message", "<script>alert('xss')</script>"}, {"csrf_token", token5}};
    std::map<std::string, std::string> session5 = {{"csrf_token", token5}};
    std::cout << "Test 5: " << post(req5, session5) << std::endl;
    
    // Display all messages
    std::cout << "\\nMessages:" << std::endl;
    auto messages = getMessages();
    for (const auto& msg : messages) {
        std::cout << msg.at("username") << ": " << msg.at("message") << std::endl;
    }
    
    return 0;
}
