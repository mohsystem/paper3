
#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <cctype>

// Rules#8, Rules#36: Use modern C++ with RAII and safe strings
// Note: This example uses a simplified HTTP framework concept
// In production, use a proper web framework with security features

// Rules#2, Rules#35, Rules#38: Maximum lengths
constexpr size_t MAX_USERNAME_LENGTH = 50;
constexpr size_t MAX_MESSAGE_LENGTH = 1000;

/**
 * Validate and sanitize input string
 * Rules#2, Rules#24, Rules#25, Rules#26
 */
std::string validateAndSanitize(const std::string& input, size_t maxLength, 
                                const std::string& fieldName) {
    // Rules#2: Check for empty input
    if (input.empty()) {
        throw std::invalid_argument("Invalid " + fieldName);
    }
    
    // Rules#26: Normalize whitespace
    std::string normalized = input;
    normalized.erase(0, normalized.find_first_not_of(" \\t\\n\\r"));
    normalized.erase(normalized.find_last_not_of(" \\t\\n\\r") + 1);
    
    // Replace multiple spaces with single space
    auto newEnd = std::unique(normalized.begin(), normalized.end(),
        [](char a, char b) { return std::isspace(a) && std::isspace(b); });
    normalized.erase(newEnd, normalized.end());
    
    // Rules#35: Bounds check before operations
    if (normalized.length() > maxLength || normalized.empty()) {
        throw std::invalid_argument("Invalid " + fieldName);
    }
    
    // Rules#25: Allow list validation
    std::regex allowedPattern("^[a-zA-Z0-9 .,!?'-]+$");
    if (!std::regex_match(normalized, allowedPattern)) {
        throw std::invalid_argument("Invalid " + fieldName);
    }
    
    return normalized;
}

/**
 * Insert message into database using parameterized query
 * Rules#32: Prevent SQL injection with prepared statements
 * This is a conceptual implementation - use actual database library
 */
bool insert_user_message_in_db(const std::string& username, const std::string& message) {
    // Rules#3: Get connection string from environment
    const char* dbPath = std::getenv("DB_PATH");
    if (dbPath == nullptr) {
        // Rules#11: Log error internally
        std::cerr << "Database configuration missing" << std::endl;
        return false;
    }
    
    try {
        // In production: Use actual database library (e.g., libpq, MySQL Connector/C++)
        // Rules#32: Use parameterized queries
        // Example pseudocode:
        // auto conn = DatabaseConnection(dbPath);
        // auto stmt = conn.prepare("INSERT INTO messages (username, message) VALUES (?, ?)");
        // stmt.bind(1, username);
        // stmt.bind(2, message);
        // stmt.execute();
        
        // Placeholder for actual database operation
        std::cout << "Inserting message (parameterized): username=" 
                  << username << ", message=" << message << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        // Rules#11: Log detailed error internally, don't expose\n        std::cerr << "Database error occurred" << std::endl;\n        return false;\n    }\n}\n\n/**\n * HTTP request handler concept\n * Rules#2, Rules#28: Validate all request inputs\n */\nclass HttpRequest {\npublic:\n    std::string method;\n    std::string path;\n    std::map<std::string, std::string> params;\n    \n    std::string getParameter(const std::string& name) const {\n        auto it = params.find(name);\n        return (it != params.end()) ? it->second : "";\n    }\n};\n\nclass HttpResponse {\npublic:\n    int statusCode = 200;\n    std::string body;\n    std::string location;\n    \n    void redirect(const std::string& url) {\n        statusCode = 302;\n        location = url;\n    }\n    \n    void sendError(int code, const std::string& message) {\n        statusCode = code;\n        body = message;\n    }\n};\n\n/**\n * POST handler for message submission\n * Rules#2, Rules#24, Rules#28\n */\nvoid post(const HttpRequest& request, HttpResponse& response) {\n    try {\n        // Rules#28: Treat all request data as untrusted\n        std::string username = request.getParameter("username");\n        std::string message = request.getParameter("message");\n        \n        // Rules#2, Rules#24: Validate inputs\n        username = validateAndSanitize(username, MAX_USERNAME_LENGTH, "username");\n        message = validateAndSanitize(message, MAX_MESSAGE_LENGTH, "message");\n        \n        // Insert into database\n        bool success = insert_user_message_in_db(username, message);\n        \n        if (!success) {\n            // Rules#11: Generic error message\n            response.sendError(500, "Unable to process request");\n            return;\n        }\n        \n        // Rules#2: Redirect to main page\n        response.redirect("/");\n        \n    } catch (const std::invalid_argument& e) {\n        // Rules#11: Don't leak details
        response.sendError(400, "Invalid input");
        
    } catch (const std::exception& e) {
        // Rules#11: Log internally, generic message
        std::cerr << "Unexpected error in post handler" << std::endl;
        response.sendError(500, "An error occurred");
    }
}

/**
 * GET handler for main page
 * Rules#31: Safe HTML generation
 */
void index(const HttpRequest& request, HttpResponse& response) {
    // Rules#31: Use constant format string
    response.body = R"(\n<!DOCTYPE html>\n<html>\n<head>\n    <title>Message Board</title>\n    <meta charset="UTF-8">\n</head>\n<body>\n    <h1>Post a Message</h1>\n    <form method="post" action="/post">\n        <label>Username: \n            <input type="text" name="username" maxlength=")" + \n            std::to_string(MAX_USERNAME_LENGTH) + R"(" required>\n        </label><br>\n        <label>Message: \n            <textarea name="message" maxlength=")" + \n            std::to_string(MAX_MESSAGE_LENGTH) + R"(" required></textarea>\n        </label><br>\n        <input type="submit" value="Post Message">\n    </form>\n</body>\n</html>\n)";
}

// Test cases
int main() {
    std::cout << "Test Case 1: Valid username and message" << std::endl;
    try {
        std::string user = validateAndSanitize("JohnDoe", MAX_USERNAME_LENGTH, "username");
        std::string msg = validateAndSanitize("Hello World!", MAX_MESSAGE_LENGTH, "message");
        std::cout << "  Passed: " << user << ", " << msg << std::endl;
    } catch (...) {
        std::cout << "  Failed" << std::endl;
    }
    
    std::cout << "\\nTest Case 2: Empty username" << std::endl;
    try {
        validateAndSanitize("", MAX_USERNAME_LENGTH, "username");
        std::cout << "  Failed: Should have thrown" << std::endl;
    } catch (const std::invalid_argument&) {
        std::cout << "  Passed: Correctly rejected" << std::endl;
    }
    
    std::cout << "\\nTest Case 3: Message exceeds max length" << std::endl;
    try {
        std::string longMsg(MAX_MESSAGE_LENGTH + 1, 'a');
        validateAndSanitize(longMsg, MAX_MESSAGE_LENGTH, "message");
        std::cout << "  Failed: Should have thrown" << std::endl;
    } catch (const std::invalid_argument&) {
        std::cout << "  Passed: Correctly rejected" << std::endl;
    }
    
    std::cout << "\\nTest Case 4: Invalid characters in input" << std::endl;
    try {
        validateAndSanitize("test<script>", MAX_USERNAME_LENGTH, "username");
        std::cout << "  Failed: Should have thrown" << std::endl;
    } catch (const std::invalid_argument&) {
        std::cout << "  Passed: Correctly rejected" << std::endl;
    }
    
    std::cout << "\\nTest Case 5: SQL injection attempt" << std::endl;
    try {
        std::string injection = "'; DROP TABLE messages; --";
        validateAndSanitize(injection, MAX_MESSAGE_LENGTH, "message");
        std::cout << "  Failed: Should have thrown" << std::endl;
    } catch (const std::invalid_argument&) {
        std::cout << "  Passed: Correctly rejected" << std::endl;
    }
    
    return 0;
}
