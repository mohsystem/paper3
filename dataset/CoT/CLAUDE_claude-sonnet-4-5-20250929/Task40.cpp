
#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include <regex>
#include <algorithm>
#include <cstring>

class Task40 {
private:
    struct TokenData {
        std::string token;
        time_t timestamp;
    };
    
    std::map<std::string, TokenData> sessionTokens;
    std::map<std::string, std::map<std::string, std::string>> userDatabase;
    static const int TOKEN_VALIDITY = 3600; // 1 hour
    
    std::string generateSecureToken() {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_int_distribution<uint64_t> dis;
        
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        for (int i = 0; i < 4; i++) {
            ss << std::setw(16) << dis(gen);
        }
        return ss.str();
    }
    
    bool constantTimeCompare(const std::string& a, const std::string& b) {
        if (a.length() != b.length()) {
            return false;
        }
        
        volatile int result = 0;
        for (size_t i = 0; i < a.length(); i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }
    
    std::string sanitizeInput(const std::string& input, size_t maxLength = 255) {
        if (input.empty()) {
            return "";
        }
        
        std::string sanitized;
        for (char c : input) {
            if (c != '<' && c != '>' && c != '"' && c != '\\'' && c != '&') {
                sanitized += c;
            }
        }
        
        // Trim whitespace
        size_t start = sanitized.find_first_not_of(" \\t\\n\\r");
        size_t end = sanitized.find_last_not_of(" \\t\\n\\r");
        
        if (start == std::string::npos) {
            return "";
        }
        
        sanitized = sanitized.substr(start, end - start + 1);
        
        if (sanitized.length() > maxLength) {
            sanitized = sanitized.substr(0, maxLength);
        }
        
        return sanitized;
    }
    
    bool validateEmail(const std::string& email) {
        if (email.empty()) {
            return false;
        }
        
        std::regex pattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        return std::regex_match(email, pattern);
    }
    
public:
    std::string generateCSRFToken(const std::string& sessionId) {
        if (sessionId.empty()) {
            throw std::invalid_argument("Session ID cannot be empty");
        }
        
        TokenData tokenData;
        tokenData.token = generateSecureToken();
        tokenData.timestamp = std::time(nullptr);
        
        sessionTokens[sessionId] = tokenData;
        
        return tokenData.token;
    }
    
    bool validateCSRFToken(const std::string& sessionId, const std::string& token) {
        if (sessionId.empty() || token.empty()) {
            return false;
        }
        
        auto it = sessionTokens.find(sessionId);
        if (it == sessionTokens.end()) {
            return false;
        }
        
        time_t currentTime = std::time(nullptr);
        if (currentTime - it->second.timestamp > TOKEN_VALIDITY) {
            sessionTokens.erase(it);
            return false;
        }
        
        return constantTimeCompare(it->second.token, token);
    }
    
    std::string updateUserSettings(const std::string& sessionId, const std::string& csrfToken,
                                   const std::string& username, const std::string& email,
                                   const std::string& theme) {
        // Input validation
        if (username.empty() || username.find_first_not_of(" \\t\\n\\r") == std::string::npos) {
            return "Error: Username cannot be empty";
        }
        
        if (!validateEmail(email)) {
            return "Error: Invalid email format";
        }
        
        if (theme.empty() || theme.find_first_not_of(" \\t\\n\\r") == std::string::npos) {
            return "Error: Theme cannot be empty";
        }
        
        // CSRF Token validation
        if (!validateCSRFToken(sessionId, csrfToken)) {
            return "Error: Invalid CSRF token. Request rejected for security reasons.";
        }
        
        // Sanitize inputs
        std::string cleanUsername = sanitizeInput(username);
        std::string cleanEmail = sanitizeInput(email);
        std::string cleanTheme = sanitizeInput(theme);
        
        // Update user settings
        std::map<std::string, std::string> settings;
        settings["username"] = cleanUsername;
        settings["email"] = cleanEmail;
        settings["theme"] = cleanTheme;
        
        userDatabase[sessionId] = settings;
        
        // Invalidate used token and generate new one
        sessionTokens.erase(sessionId);
        std::string newToken = generateCSRFToken(sessionId);
        
        return "Success: Settings updated. New CSRF token: " + newToken;
    }
};

int main() {
    std::cout << "=== CSRF-Protected User Settings Update Demo ===\\n\\n";
    
    Task40 app;
    
    // Test Case 1: Valid update with correct CSRF token
    std::cout << "Test Case 1: Valid update\\n";
    std::string session1 = "session_12345";
    std::string token1 = app.generateCSRFToken(session1);
    std::cout << "Generated CSRF Token: " << token1 << "\\n";
    std::string result1 = app.updateUserSettings(session1, token1, "john_doe", "john@example.com", "dark");
    std::cout << result1 << "\\n\\n";
    
    // Test Case 2: Invalid CSRF token
    std::cout << "Test Case 2: Invalid CSRF token\\n";
    std::string session2 = "session_67890";
    app.generateCSRFToken(session2);
    std::string result2 = app.updateUserSettings(session2, "invalid_token", "jane_doe", "jane@example.com", "light");
    std::cout << result2 << "\\n\\n";
    
    // Test Case 3: Token reuse
    std::cout << "Test Case 3: Token reuse attempt\\n";
    std::string session3 = "session_11111";
    std::string token3 = app.generateCSRFToken(session3);
    app.updateUserSettings(session3, token3, "alice", "alice@example.com", "blue");
    std::string result3 = app.updateUserSettings(session3, token3, "alice_updated", "alice2@example.com", "red");
    std::cout << result3 << "\\n\\n";
    
    // Test Case 4: Invalid email
    std::cout << "Test Case 4: Invalid email format\\n";
    std::string session4 = "session_22222";
    std::string token4 = app.generateCSRFToken(session4);
    std::string result4 = app.updateUserSettings(session4, token4, "bob", "invalid-email", "green");
    std::cout << result4 << "\\n\\n";
    
    // Test Case 5: Empty username
    std::cout << "Test Case 5: Empty username\\n";
    std::string session5 = "session_33333";
    std::string token5 = app.generateCSRFToken(session5);
    std::string result5 = app.updateUserSettings(session5, token5, "", "test@example.com", "yellow");
    std::cout << result5 << "\\n\\n";
    
    return 0;
}
