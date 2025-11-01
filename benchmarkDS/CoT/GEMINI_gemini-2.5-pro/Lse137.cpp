#include <iostream>
#include <string>
#include <map>

// In a real application, user data would come from a secure database.
// Passwords should be securely hashed (e.g., using Argon2, bcrypt), not stored in plain text.
struct UserData {
    std::string password; // NOTE: In a real app, this would be a hash.
    std::string session;
    std::string role;
};

// Mock database
std::map<std::string, UserData> userDatabase = {
    {"admin", {"pass123", "adminsession123", "moderator"}},
    {"user", {"password", "usersession456", "user"}}
};

/**
 * @brief Authenticates a user and checks if they have the 'moderator' role.
 * 
 * SECURITY NOTE: In a real-world scenario:
 * 1. Passwords must be compared using a secure, constant-time hash comparison function to prevent timing attacks.
 * 2. This function should consult a database or a secure authentication service.
 * 3. Session tokens should be long, random, and managed securely.
 * 
 * @param username The user's name.
 * @param password The user's plain-text password.
 * @param session  The user's session token.
 * @return true if the user is authenticated and is a moderator, false otherwise.
 */
bool auth_mod(const std::string& username, const std::string& password, const std::string& session) {
    auto it = userDatabase.find(username);
    
    if (it != userDatabase.end()) {
        const UserData& userData = it->second;
        
        // NOTE: Plain text password comparison is insecure.
        bool isPasswordCorrect = (userData.password == password);
        bool isSessionCorrect = (userData.session == session);
        bool isModerator = (userData.role == "moderator");
        
        return isPasswordCorrect && isSessionCorrect && isModerator;
    }
    
    return false;
}

/**
 * @brief Checks if a user is a moderator based on parameters.
 * 
 * SECURITY NOTE: Getting credentials from a URL is extremely insecure (CWE-598).
 * Sensitive data like passwords and session tokens should be sent in the body
 * of a POST request over HTTPS.
 * 
 * @param username The user's name.
 * @param password The user's plain-text password.
 * @param session  The user's session token.
 * @return true if the user is a moderator, false otherwise.
 */
bool check_mod(const std::string& username, const std::string& password, const std::string& session) {
    return auth_mod(username, password, session);
}

int main() {
    // This class name is for consistency with the Java example, though not required in C++.
    class Lse137 {}; 

    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid moderator credentials
    bool test1 = check_mod("admin", "pass123", "adminsession123");
    std::cout << "Test Case 1 (Valid Moderator): " << (test1 ? "Passed" : "Failed") << std::endl;

    // Test Case 2: Valid user but not a moderator
    bool test2 = check_mod("user", "password", "usersession456");
    std::cout << "Test Case 2 (Not a Moderator): " << (!test2 ? "Passed" : "Failed") << std::endl;

    // Test Case 3: Correct username, wrong password
    bool test3 = check_mod("admin", "wrongpassword", "adminsession123");
    std::cout << "Test Case 3 (Wrong Password): " << (!test3 ? "Passed" : "Failed") << std::endl;

    // Test Case 4: Correct credentials, wrong session
    bool test4 = check_mod("admin", "pass123", "wrongsession");
    std::cout << "Test Case 4 (Wrong Session): " << (!test4 ? "Passed" : "Failed") << std::endl;

    // Test Case 5: Non-existent user
    bool test5 = check_mod("unknown", "password", "somesession");
    std::cout << "Test Case 5 (Non-existent User): " << (!test5 ? "Passed" : "Failed") << std::endl;

    return 0;
}