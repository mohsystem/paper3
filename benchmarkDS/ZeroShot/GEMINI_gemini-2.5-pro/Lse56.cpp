#include <iostream>
#include <string>
#include <map>

// --- Mock User Data Structure and Database ---
struct User {
    std::string username;
    std::string email;
    std::string passwordHash;
};

// In a real application, this would be a database.
std::map<std::string, User> userDatabase;
// In a real application, session management would be more robust.
User* currentUser = nullptr;


// --- Security Utilities (Mocks) ---

// MOCK HASHING: NOT FOR PRODUCTION USE.
// Use a well-vetted cryptography library (like OpenSSL for SHA-256, or a dedicated
// library for Argon2/BCrypt) in a real application.
std::string hashPassword(const std::string& password) {
    // This is a simple, insecure "hash" for demonstration only.
    std::string hash = "hashed_";
    for (char c : password) {
        hash += std::to_string(static_cast<int>(c) % 10);
    }
    return hash;
}

// MOCK VERIFICATION: NOT FOR PRODUCTION USE.
// This comparison should be done in a constant-time manner to prevent timing attacks.
bool verifyPassword(const std::string& plainPassword, const std::string& storedHash) {
    if (storedHash.empty() || plainPassword.empty()) {
        return false;
    }
    return hashPassword(plainPassword) == storedHash;
}

// Basic email format validation
bool isValidEmailFormat(const std::string& email) {
    // A simple check, a more robust regex would be used in production.
    return email.find('@') != std::string::npos && email.find('.') != std::string::npos;
}


// --- Core Functionality ---

bool login(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end() && verifyPassword(password, it->second.passwordHash)) {
        currentUser = &(it->second);
        return true;
    }
    currentUser = nullptr;
    return false;
}

void logout() {
    currentUser = nullptr;
}

/**
 * @brief Changes the user's email address after performing security checks.
 * @param oldEmail The user's current email address for verification.
 * @param newEmail The desired new email address.
 * @param password The user's current password for authorization.
 * @return A string indicating the result of the operation.
 */
std::string changeEmail(const std::string& oldEmail, const std::string& newEmail, const std::string& password) {
    // 1. Check if a user is logged in
    if (currentUser == nullptr) {
        return "Error: You must be logged in to change your email.";
    }

    // 2. Verify the provided password is correct for the logged-in user
    if (!verifyPassword(password, currentUser->passwordHash)) {
        return "Error: Incorrect password.";
    }

    // 3. Verify the provided old email matches the one on record
    if (currentUser->email != oldEmail) {
        return "Error: The old email address does not match our records.";
    }

    // 4. Check if the new email is the same as the old email
    if (oldEmail == newEmail) {
        return "Error: New email cannot be the same as the old email.";
    }

    // 5. Validate the new email format
    if (!isValidEmailFormat(newEmail)) {
        return "Error: Invalid new email format.";
    }
    
    // All checks passed, update the email
    currentUser->email = newEmail;
    return "Success: Email has been updated to " + newEmail;
}


// --- Main function with test cases ---
int main() {
    // Setup: Create a user and add to the mock database
    std::string username = "testuser";
    std::string initial_email = "user@example.com";
    std::string initial_password = "Password123!";
    userDatabase[username] = {username, initial_email, hashPassword(initial_password)};

    std::cout << "--- Test Case 1: Successful Email Change ---" << std::endl;
    login(username, initial_password);
    std::cout << "Login successful for: " << (currentUser ? currentUser->username : "none") << std::endl;
    std::string result1 = changeEmail(initial_email, "new.user@example.com", initial_password);
    std::cout << "Result: " << result1 << std::endl;
    std::cout << "Current email: " << (currentUser ? currentUser->email : "N/A") << std::endl;
    logout();
    std::cout << "\n-------------------------------------------------\n" << std::endl;

    // Reset user email for next tests
    userDatabase[username].email = initial_email;

    std::cout << "--- Test Case 2: Failure - Not Logged In ---" << std::endl;
    std::cout << "Current user is logged out." << std::endl;
    std::string result2 = changeEmail(initial_email, "another.email@example.com", initial_password);
    std::cout << "Result: " << result2 << std::endl;
    std::cout << "\n-------------------------------------------------\n" << std::endl;

    std::cout << "--- Test Case 3: Failure - Incorrect Password ---" << std::endl;
    login(username, initial_password);
    std::cout << "Login successful for: " << (currentUser ? currentUser->username : "none") << std::endl;
    std::string result3 = changeEmail(initial_email, "another.email@example.com", "WrongPassword!");
    std::cout << "Result: " << result3 << std::endl;
    std::cout << "Email was not changed: " << currentUser->email << std::endl;
    logout();
    std::cout << "\n-------------------------------------------------\n" << std::endl;

    std::cout << "--- Test Case 4: Failure - Incorrect Old Email ---" << std::endl;
    login(username, initial_password);
    std::cout << "Login successful for: " << (currentUser ? currentUser->username : "none") << std::endl;
    std::string result4 = changeEmail("wrong.old@example.com", "another.email@example.com", initial_password);
    std::cout << "Result: " << result4 << std::endl;
    std::cout << "Email was not changed: " << currentUser->email << std::endl;
    logout();
    std::cout << "\n-------------------------------------------------\n" << std::endl;

    std::cout << "--- Test Case 5: Failure - New Email is Same as Old ---" << std::endl;
    login(username, initial_password);
    std::cout << "Login successful for: " << (currentUser ? currentUser->username : "none") << std::endl;
    std::string result5 = changeEmail(initial_email, initial_email, initial_password);
    std::cout << "Result: " << result5 << std::endl;
    std::cout << "Email was not changed: " << currentUser->email << std::endl;
    logout();
    std::cout << "\n-------------------------------------------------\n" << std::endl;

    return 0;
}