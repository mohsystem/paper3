
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <algorithm>

class Lse42 {
private:
    static std::map<std::string, std::map<std::string, std::string>> sessionStore;
    
    static std::string sanitize(const std::string& input) {
        if (input.empty()) return "";
        std::string result = input;
        
        // Remove potentially dangerous characters
        result = std::regex_replace(result, std::regex("[<>\\"'&]"), "");
        result = std::regex_replace(result, std::regex("javascript:", std::regex::icase), "");
        result = std::regex_replace(result, std::regex("on\\\\w+=", std::regex::icase), "");
        
        return result;
    }
    
    static std::string maskSSN(const std::string& ssn) {
        if (ssn.empty() || ssn.length() < 4) return "***";
        return "XXX-XX-" + ssn.substr(ssn.length() - 4);
    }
    
public:
    static std::string createSession(const std::string& sessionId, 
                                     const std::string& username,
                                     const std::string& email,
                                     const std::string& ssn) {
        std::map<std::string, std::string> sessionData;
        sessionData["username"] = sanitize(username);
        sessionData["email"] = sanitize(email);
        sessionData["ssn"] = maskSSN(ssn);
        sessionData["authenticated"] = "true";
        sessionStore[sessionId] = sessionData;
        return sessionId;
    }
    
    static std::string info(const std::string& sessionId) {
        if (sessionId.empty() || sessionStore.find(sessionId) == sessionStore.end()) {
            return "Error: Invalid or expired session. Please log in.";
        }
        
        auto sessionData = sessionStore[sessionId];
        
        if (sessionData["authenticated"] != "true") {
            return "Error: Unauthorized access.";
        }
        
        std::string username = sessionData.count("username") ? sessionData["username"] : "N/A";
        std::string email = sessionData.count("email") ? sessionData["email"] : "N/A";
        std::string ssn = sessionData.count("ssn") ? sessionData["ssn"] : "N/A";
        
        return "User Information:\\nUsername: " + username + 
               "\\nEmail: " + email + 
               "\\nSSN: " + ssn;
    }
};

// Initialize static member
std::map<std::string, std::map<std::string, std::string>> Lse42::sessionStore;

int main() {
    std::cout << "Testing /info route with session management\\n\\n";
    
    // Test case 1: Valid session
    Lse42::createSession("sess001", "john_doe", "john@example.com", "123456789");
    std::cout << "Test 1 - Valid session:\\n";
    std::cout << Lse42::info("sess001") << "\\n\\n";
    
    // Test case 2: Another valid session
    Lse42::createSession("sess002", "jane_smith", "jane@example.com", "987654321");
    std::cout << "Test 2 - Another valid session:\\n";
    std::cout << Lse42::info("sess002") << "\\n\\n";
    
    // Test case 3: Invalid session ID
    std::cout << "Test 3 - Invalid session:\\n";
    std::cout << Lse42::info("invalid_session") << "\\n\\n";
    
    // Test case 4: Empty session ID
    std::cout << "Test 4 - Empty session:\\n";
    std::cout << Lse42::info("") << "\\n\\n";
    
    // Test case 5: XSS attempt in username
    Lse42::createSession("sess003", "<script>alert('xss')</script>user", 
                        "test@example.com", "555667777");
    std::cout << "Test 5 - XSS attempt (sanitized):\\n";
    std::cout << Lse42::info("sess003") << "\\n";
    
    return 0;
}
