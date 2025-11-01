
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <cstring>
#include <algorithm>

// Simple session storage (in production, use proper session management)
static std::map<std::string, std::map<std::string, std::string>> sessionStore;

/**
 * Sanitizes output to prevent injection attacks.
 * SECURITY: Removes potentially dangerous characters.
 */
std::string sanitizeForOutput(const std::string& input) {
    if (input.empty()) {
        return "";
    }
    
    std::string result = input;
    // Remove potentially dangerous characters
    const char* dangerous = "<>&\\"'";
    result.erase(std::remove_if(result.begin(), result.end(),
        [dangerous](char c) {
            return std::strchr(dangerous, c) != nullptr;
        }), result.end());
    
    return result;
}

/**
 * Masks email to show only first character and domain.
 * SECURITY: Prevents full email exposure in responses.
 */
std::string maskEmail(const std::string& email) {
    if (email.empty()) {
        return "***";
    }
    
    size_t atPos = email.find('@');
    if (atPos == std::string::npos || atPos == 0) {
        return "***";
    }
    
    return std::string(1, email[0]) + "***@" + email.substr(atPos + 1);
}

/**
 * Masks SSN to show only last 4 digits.
 * SECURITY: Prevents full SSN exposure in responses.
 */
std::string maskSsn(const std::string& ssn) {
    if (ssn.length() < 4) {
        return "***";
    }
    
    return "XXX-XX-" + ssn.substr(ssn.length() - 4);
}

/**
 * Returns user information from session.
 * SECURITY: Never log sensitive data like SSN or email.
 * Returns only redacted/masked information to prevent exposure.
 */
std::string info(const std::string& sessionId) {
    // Validate session ID input - reject if empty
    if (sessionId.empty()) {
        // Return generic error, don't leak session details\n        return "Error: Invalid session";\n    }\n    \n    // Check if session exists\n    auto sessionIt = sessionStore.find(sessionId);\n    if (sessionIt == sessionStore.end()) {\n        // Session not found - return generic error\n        return "Error: Session not found";\n    }\n    \n    const auto& sessionData = sessionIt->second;\n    \n    // Retrieve session variables - treat all as untrusted\n    std::string username = sessionData.count("username") ? sessionData.at("username") : "";\n    std::string email = sessionData.count("email") ? sessionData.at("email") : "";\n    std::string ssn = sessionData.count("ssn") ? sessionData.at("ssn") : "";\n    \n    // SECURITY: Redact sensitive information (SSN, email)\n    // Never return full SSN or email in responses\n    std::string maskedEmail = maskEmail(email);\n    std::string maskedSsn = maskSsn(ssn);\n    std::string sanitizedUsername = sanitizeForOutput(username);\n    \n    // Return sanitized data using safe string construction\n    return "Username: " + sanitizedUsername + ", Email: " + maskedEmail + ", SSN: " + maskedSsn;\n}\n\n// Helper function to setup test sessions\nvoid setupTestSession(const std::string& sessionId, const std::string& username, \n                      const std::string& email, const std::string& ssn) {\n    std::map<std::string, std::string> data;\n    data["username"] = username;\n    data["email"] = email;\n    data["ssn"] = ssn;\n    sessionStore[sessionId] = data;\n}\n\nint main() {\n    std::cout << "=== Test Cases ===" << std::endl;\n    \n    // Test case 1: Valid session\n    setupTestSession("session1", "john_doe", "john@example.com", "123456789");\n    std::cout << "Test 1: " << info("session1") << std::endl;\n    \n    // Test case 2: Another valid session\n    setupTestSession("session2", "jane_smith", "jane@test.org", "987654321");\n    std::cout << "Test 2: " << info("session2") << std::endl;\n    \n    // Test case 3: Invalid/non-existent session\n    std::cout << "Test 3: " << info("invalid_session") << std::endl;\n    \n    // Test case 4: Empty session ID\n    std::cout << "Test 4: " << info("") << std::endl;\n    \n    // Test case 5: Session with special characters in username\n    setupTestSession("session3", "<script>alert('xss')</script>", "user@domain.com", "111223333");\n    std::cout << "Test 5: " << info("session3") << std::endl;\n    \n    std::cout << "==================" << std::endl;
    
    return 0;
}
