
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <regex>
#include <iomanip>
#include <openssl/hmac.h>
#include <openssl/rand.h>

const size_t MAX_USERNAME_LENGTH = 50;
const size_t MAX_EMAIL_LENGTH = 100;
const size_t MAX_THEME_LENGTH = 20;

std::map<std::string, std::string> sessionTokens;
std::map<std::string, std::map<std::string, std::string>> userSettings;
unsigned char SECRET_KEY[32];

std::string base64_encode(const unsigned char* buffer, size_t length) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    std::string result;
    
    for (size_t i = 0; i < length; i += 3) {
        unsigned char b1 = buffer[i];
        unsigned char b2 = (i + 1 < length) ? buffer[i + 1] : 0;
        unsigned char b3 = (i + 2 < length) ? buffer[i + 2] : 0;
        
        result += base64_chars[b1 >> 2];
        result += base64_chars[((b1 & 0x03) << 4) | (b2 >> 4)];
        if (i + 1 < length) result += base64_chars[((b2 & 0x0F) << 2) | (b3 >> 6)];
        if (i + 2 < length) result += base64_chars[b3 & 0x3F];
    }
    
    return result;
}

std::string generateRandomToken(size_t length) {
    unsigned char buffer[64];
    if (length > sizeof(buffer)) length = sizeof(buffer);
    
    if (RAND_bytes(buffer, length) != 1) {
        throw std::runtime_error("Failed to generate random bytes");
    }
    
    return base64_encode(buffer, length);
}

std::string generateCSRFToken(const std::string& sessionId) {
    std::string token = generateRandomToken(32);
    
    std::string data = sessionId + ":" + token;
    unsigned char hmacResult[EVP_MAX_MD_SIZE];
    unsigned int hmacLen = 0;
    
    HMAC(EVP_sha256(), SECRET_KEY, sizeof(SECRET_KEY),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         hmacResult, &hmacLen);
    
    std::string signature = base64_encode(hmacResult, hmacLen);
    std::string csrfToken = token + "." + signature;
    sessionTokens[sessionId] = csrfToken;
    
    return csrfToken;
}

bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.empty() || b.empty() || a.length() != b.length()) {
        return false;
    }
    
    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    }
    return result == 0;
}

bool validateCSRFToken(const std::string& sessionId, const std::string& token) {
    if (sessionId.empty() || token.empty()) {
        return false;
    }
    
    auto it = sessionTokens.find(sessionId);
    if (it == sessionTokens.end()) {
        return false;
    }
    
    return constantTimeEquals(it->second, token);
}

std::string sanitizeInput(const std::string& input, size_t maxLength) {
    if (input.empty()) return "";
    
    std::string sanitized = input;
    sanitized.erase(0, sanitized.find_first_not_of(" \\t\\n\\r"));
    sanitized.erase(sanitized.find_last_not_of(" \\t\\n\\r") + 1);
    
    if (sanitized.length() > maxLength) {
        sanitized = sanitized.substr(0, maxLength);
    }
    
    sanitized.erase(std::remove_if(sanitized.begin(), sanitized.end(),
        [](char c) { return c == '<' || c == '>' || c == '"' || c == '\\'' || c == '&'; }),
        sanitized.end());
    
    return sanitized;
}

bool isValidEmail(const std::string& email) {
    if (email.empty() || email.length() > MAX_EMAIL_LENGTH) {
        return false;
    }
    
    std::regex emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$)");
    return std::regex_match(email, emailRegex);
}

std::string updateUserSettings(const std::string& sessionId, const std::string& csrfToken,
                              const std::string& username, const std::string& email,
                              const std::string& theme) {
    if (!validateCSRFToken(sessionId, csrfToken)) {
        return "Error: Invalid CSRF token";
    }
    
    std::string cleanUsername = sanitizeInput(username, MAX_USERNAME_LENGTH);
    std::string cleanEmail = sanitizeInput(email, MAX_EMAIL_LENGTH);
    std::string cleanTheme = sanitizeInput(theme, MAX_THEME_LENGTH);
    
    if (cleanUsername.empty()) {
        return "Error: Username cannot be empty";
    }
    
    if (!isValidEmail(cleanEmail)) {
        return "Error: Invalid email format";
    }
    
    if (cleanTheme != "light" && cleanTheme != "dark" && cleanTheme != "auto") {
        return "Error: Invalid theme selection";
    }
    
    userSettings[sessionId]["username"] = cleanUsername;
    userSettings[sessionId]["email"] = cleanEmail;
    userSettings[sessionId]["theme"] = cleanTheme;
    
    sessionTokens.erase(sessionId);
    
    return "Success: Settings updated successfully";
}

int main() {
    if (RAND_bytes(SECRET_KEY, sizeof(SECRET_KEY)) != 1) {
        std::cerr << "Failed to initialize secret key" << std::endl;
        return 1;
    }
    
    std::cout << "Test Case 1: Valid CSRF token" << std::endl;
    std::string session1 = "session1";
    std::string token1 = generateCSRFToken(session1);
    std::string result1 = updateUserSettings(session1, token1, "john_doe", "john@example.com", "dark");
    std::cout << result1 << std::endl;
    
    std::cout << "\\nTest Case 2: Invalid CSRF token" << std::endl;
    std::string session2 = "session2";
    generateCSRFToken(session2);
    std::string result2 = updateUserSettings(session2, "invalid_token", "jane_doe", "jane@example.com", "light");
    std::cout << result2 << std::endl;
    
    std::cout << "\\nTest Case 3: Invalid email format" << std::endl;
    std::string session3 = "session3";
    std::string token3 = generateCSRFToken(session3);
    std::string result3 = updateUserSettings(session3, token3, "bob", "invalid-email", "auto");
    std::cout << result3 << std::endl;
    
    std::cout << "\\nTest Case 4: Empty username" << std::endl;
    std::string session4 = "session4";
    std::string token4 = generateCSRFToken(session4);
    std::string result4 = updateUserSettings(session4, token4, "", "alice@example.com", "light");
    std::cout << result4 << std::endl;
    
    std::cout << "\\nTest Case 5: Invalid theme" << std::endl;
    std::string session5 = "session5";
    std::string token5 = generateCSRFToken(session5);
    std::string result5 = updateUserSettings(session5, token5, "charlie", "charlie@example.com", "invalid_theme");
    std::cout << result5 << std::endl;
    
    return 0;
}
