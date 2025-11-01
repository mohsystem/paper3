#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iomanip>
#include <random>

// --- Simulated Database ---

struct User {
    std::string username;
    std::string hashedPassword;
    std::string role;
};

// Use std::map to simulate a database table
std::map<std::string, User> userDatabase;
std::map<std::string, std::string> sessionDatabase; // SessionToken -> Username

// --- Security Utilities ---

// SECURITY NOTE: This is a *demonstration* hash function. In a real application,
// use a strong, salted, and standardized library like OpenSSL for hashing,
// and algorithms like BCrypt or Argon2.
std::string hashPassword(const std::string& password) {
    std::hash<std::string> hasher;
    size_t hashed = hasher(password + "a_simple_salt"); // Simple salt for demonstration
    std::stringstream ss;
    ss << std::hex << hashed;
    return ss.str();
}

// Generates a random alphanumeric string to be used as a session token.
// In a real application, ensure the random source is cryptographically secure.
std::string generateSessionToken(size_t length) {
    const std::string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, chars.size() - 1);
    
    std::string token;
    for (size_t i = 0; i < length; ++i) {
        token += chars[distribution(generator)];
    }
    return token;
}

// Constant-time string comparison to prevent timing attacks.
bool secure_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    int result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// --- Simulated Authentication and Authorization Logic ---

std::string login(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end()) {
        const User& user = it->second;
        std::string providedPasswordHash = hashPassword(password);
        if (secure_compare(user.hashedPassword, providedPasswordHash)) {
            std::string sessionToken = generateSessionToken(32);
            sessionDatabase[sessionToken] = username;
            return sessionToken;
        }
    }
    return ""; // Return empty string on failure
}

bool auth_mod(const std::string& session) {
    if (session.empty()) {
        return false;
    }
    auto session_it = sessionDatabase.find(session);
    if (session_it != sessionDatabase.end()) {
        std::string username = session_it->second;
        auto user_it = userDatabase.find(username);
        if (user_it != userDatabase.end()) {
            return user_it->second.role == "moderator";
        }
    }
    return false;
}

bool check_mod(const std::string& username, const std::string& password, const std::string& session) {
    // SECURITY NOTE: After login, authorization should be based on a session token,
    // not by re-submitting credentials. The username and password parameters are
    // ignored here to follow secure design principles.
    return auth_mod(session);
}

// --- Main function with Test Cases ---
int main() {
    // 1. Setup: Populate user database
    userDatabase["admin"] = {"admin", hashPassword("adminpass123"), "moderator"};
    userDatabase["bob"] = {"bob", hashPassword("bobspassword"), "user"};
    userDatabase["charlie"] = {"charlie", hashPassword("charP@ss!"), "moderator"};

    // 2. Simulate logins to get session tokens
    std::string adminSession = login("admin", "adminpass123");
    std::string bobSession = login("bob", "bobspassword");
    std::string failedLoginSession = login("admin", "wrongpassword"); // Will be empty

    std::cout << "--- Test Cases for check_mod() ---" << std::endl;

    // Test Case 1: Valid moderator user
    bool test1 = check_mod("admin", "adminpass123", adminSession);
    std::cout << "Test Case 1 (Admin is moderator): " << (test1 ? "Passed" : "Failed") << std::endl;

    // Test Case 2: Valid non-moderator user
    bool test2 = check_mod("bob", "bobspassword", bobSession);
    std::cout << "Test Case 2 (Bob is not moderator): " << (!test2 ? "Passed" : "Failed") << std::endl;

    // Test Case 3: Invalid session token
    bool test3 = check_mod("guest", "pass", "invalid-session-token-12345");
    std::cout << "Test Case 3 (Invalid session): " << (!test3 ? "Passed" : "Failed") << std::endl;

    // Test Case 4: Empty session token (from failed login)
    bool test4 = check_mod("admin", "wrongpassword", failedLoginSession);
    std::cout << "Test Case 4 (Empty session from failed login): " << (!test4 ? "Passed" : "Failed") << std::endl;

    // Test Case 5: Explicitly empty session token
    bool test5 = check_mod("user", "pass", "");
    std::cout << "Test Case 5 (Explicitly empty session): " << (!test5 ? "Passed" : "Failed") << std::endl;

    return 0;
}