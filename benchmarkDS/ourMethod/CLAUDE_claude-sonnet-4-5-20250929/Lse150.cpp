
#include <iostream>
#include <string>
#include <regex>
#include <stdexcept>
#include <memory>
#include <cstring>
#include <openssl/rand.h>
#include <sqlite3.h>

// Rules#2: Maximum lengths to prevent resource exhaustion
constexpr size_t MAX_USERNAME_LENGTH = 50;
constexpr size_t MAX_MESSAGE_LENGTH = 500;

// Rules#11: Generic error message
constexpr const char* GENERIC_ERROR = "Unable to process request";

/**
 * Rules#2: Validates username input
 * Checks type, length, and allowed character set
 */
bool validateUsername(const std::string& username) {
    // Rules#2: Check for empty or too long
    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    // Rules#2: Allow only alphanumeric, underscore, hyphen
    static const std::regex pattern("^[a-zA-Z0-9_-]+$");
    return std::regex_match(username, pattern);
}

/**
 * Rules#2: Validates message input
 * Checks type, length
 */
bool validateMessage(const std::string& message) {
    // Rules#2: Check for empty or too long
    if (message.empty() || message.length() > MAX_MESSAGE_LENGTH) {
        return false;
    }
    return true;
}

/**
 * Rules#5: Generate CSRF token using OpenSSL CSPRNG
 */
std::string generateCsrfToken() {
    // Rules#5: Use CSPRNG (OpenSSL's RAND_bytes)\n    unsigned char buffer[32];\n    if (RAND_bytes(buffer, sizeof(buffer)) != 1) {\n        throw std::runtime_error("Failed to generate random token");\n    }\n    \n    // Convert to hex string\n    std::string token;\n    token.reserve(64);\n    for (size_t i = 0; i < sizeof(buffer); ++i) {\n        char hex[3];\n        snprintf(hex, sizeof(hex), "%02x", buffer[i]);\n        token += hex;\n    }\n    return token;\n}\n\n/**\n * Rules#32: Parameterized query to prevent SQL injection\n * Rules#8: Proper error handling and resource cleanup\n */\nvoid insertUserMessageInDb(sqlite3* db, const std::string& username, const std::string& message) {\n    // Rules#32: Use prepared statement with parameters\n    const char* sql = "INSERT INTO messages (username, message, created_at) VALUES (?, ?, datetime('now'))";\n    \n    sqlite3_stmt* stmt = nullptr;\n    \n    // Rules#8: Check preparation result\n    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {\n        throw std::runtime_error(GENERIC_ERROR);\n    }\n    \n    // Rules#8: Use unique_ptr for automatic cleanup\n    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmtGuard(stmt, sqlite3_finalize);\n    \n    // Rules#32: Bind parameters safely - SQLite handles escaping\n    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK ||\n        sqlite3_bind_text(stmt, 2, message.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {\n        throw std::runtime_error(GENERIC_ERROR);\n    }\n    \n    // Rules#8: Check execution result\n    if (sqlite3_step(stmt) != SQLITE_DONE) {\n        throw std::runtime_error(GENERIC_ERROR);\n    }\n}\n\n/**\n * Rules#24: HTML escaping to prevent XSS\n */\nstd::string escapeHtml(const std::string& input) {\n    std::string output;\n    output.reserve(input.length() * 1.2);\n    \n    for (char c : input) {\n        switch (c) {\n            case '&': output += "&amp;"; break;\n            case '<': output += "&lt;"; break;\n            case '>': output += "&gt;"; break;\n            case '"': output += "&quot;"; break;\n            case '\\'': output += "&#x27;"; break;\n            default: output += c; break;\n        }\n    }\n    return output;\n}\n\n/**\n * Rules#9: Secure memory cleanup for sensitive data\n */\nclass SecureString {\nprivate:\n    std::string data;\n    \npublic:\n    SecureString() = default;\n    explicit SecureString(const std::string& s) : data(s) {}\n    \n    ~SecureString() {\n        // Rules#9: Explicitly zero memory before destruction\n        if (!data.empty()) {\n            // Use volatile to prevent compiler optimization\n            volatile char* p = const_cast<volatile char*>(data.data());\n            for (size_t i = 0; i < data.size(); ++i) {\n                p[i] = 0;\n            }\n        }\n    }\n    \n    const std::string& get() const { return data; }\n    void set(const std::string& s) { data = s; }\n};\n\n/**\n * Simulated POST handler for message submission\n * Rules#24-28: Comprehensive input validation\n */\nclass MessageHandler {\nprivate:\n    sqlite3* db;\n    SecureString csrfToken;\n    \npublic:\n    MessageHandler() : db(nullptr) {\n        // Initialize database\n        if (sqlite3_open("messages.db", &db) != SQLITE_OK) {\n            throw std::runtime_error("Failed to open database");\n        }\n        \n        // Create table\n        const char* createTable = \n            "CREATE TABLE IF NOT EXISTS messages ("\n            "id INTEGER PRIMARY KEY AUTOINCREMENT,"\n            "username TEXT NOT NULL,"\n            "message TEXT NOT NULL,"\n            "created_at TEXT NOT NULL)";\n        \n        char* errMsg = nullptr;\n        if (sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {\n            std::string error = errMsg;\n            sqlite3_free(errMsg);\n            sqlite3_close(db);\n            throw std::runtime_error(error);\n        }\n        \n        // Rules#5: Generate CSRF token\n        csrfToken.set(generateCsrfToken());\n    }\n    \n    ~MessageHandler() {\n        if (db) {\n            sqlite3_close(db);\n        }\n    }\n    \n    /**\n     * Rules#24: Validate CSRF token\n     */\n    bool validateCsrfToken(const std::string& requestToken) {\n        // Rules#24: Constant-time comparison\n        const std::string& sessionToken = csrfToken.get();\n        if (sessionToken.length() != requestToken.length()) {\n            return false;\n        }\n        \n        volatile int result = 0;\n        for (size_t i = 0; i < sessionToken.length(); ++i) {\n            result |= sessionToken[i] ^ requestToken[i];\n        }\n        return result == 0;\n    }\n    \n    /**\n     * POST handler\n     */\n    void post(const std::string& username, const std::string& message, const std::string& csrfTokenParam) {\n        // Rules#24-28: Validate CSRF token first\n        if (!validateCsrfToken(csrfTokenParam)) {\n            std::cerr << "CSRF validation failed" << std::endl;\n            throw std::runtime_error(GENERIC_ERROR);\n        }\n        \n        // Rules#24-25: Validate inputs\n        if (!validateUsername(username)) {\n            throw std::invalid_argument("Invalid username");\n        }\n        \n        if (!validateMessage(message)) {\n            throw std::invalid_argument("Invalid message");\n        }\n        \n        try {\n            // Rules#32: Use parameterized query\n            insertUserMessageInDb(db, username, message);\n            std::cout << "Message posted successfully" << std::endl;\n        } catch (const std::exception& e) {\n            // Rules#10-11: Log securely, return generic error\n            std::cerr << "Database error" << std::endl;\n            throw std::runtime_error(GENERIC_ERROR);\n        }\n    }\n    \n    /**\n     * Generate HTML form\n     */\n    std::string generateForm() const {\n        std::string form = "<!DOCTYPE html><html><head><title>Message Board</title></head><body>";\n        form += "<h1>Post a Message</h1>";\n        form += "<form method='POST' action='/post'>";\n        form += "<input type='hidden' name='csrf_token' value='" + escapeHtml(csrfToken.get()) + "'>";\n        form += "<label>Username: <input type='text' name='username' maxlength='";\n        form += std::to_string(MAX_USERNAME_LENGTH) + "' required></label><br>";\n        form += "<label>Message: <textarea name='message' maxlength='";\n        form += std::to_string(MAX_MESSAGE_LENGTH) + "' required></textarea></label><br>";\n        form += "<button type='submit'>Submit</button>";\n        form += "</form></body></html>";\n        return form;\n    }\n    \n    const std::string& getCsrfToken() const {\n        return csrfToken.get();\n    }\n};\n\n// Test cases\nint main() {\n    try {\n        // Test 1: Valid username\n        std::cout << "Test 1: " << (validateUsername("john_doe") ? "true" : "false") << std::endl;\n        \n        // Test 2: Invalid username with special chars\n        std::cout << "Test 2: " << (validateUsername("john@doe") ? "true" : "false") << std::endl;\n        \n        // Test 3: Username too long\n        std::cout << "Test 3: " << (validateUsername(std::string(100, 'a')) ? "true" : "false") << std::endl;\n        \n        // Test 4: Valid message\n        std::cout << "Test 4: " << (validateMessage("Hello world!") ? "true" : "false") << std::endl;\n        \n        // Test 5: Message too long\n        std::cout << "Test 5: " << (validateMessage(std::string(600, 'a')) ? "true" : "false") << std::endl;\n        \n    } catch (const std::exception& e) {\n        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
