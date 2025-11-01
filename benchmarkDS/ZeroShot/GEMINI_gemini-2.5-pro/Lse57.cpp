#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <algorithm> // For std::reverse

// --- User Model ---
struct User {
    std::string username;
    std::string email;
    std::string hashedPassword; // In a real app, this would be a strong, salted hash.
};

// --- Data Structures to Simulate Database and Sessions ---
std::map<std::string, User> userDatabase;
std::unordered_set<std::string> emailDatabase;
std::unordered_set<std::string> loggedInUsers;

// --- Security Simulation ---
/**
 * A simple, insecure password hashing simulation.
 * WARNING: Do NOT use this in production. Use a strong library like OpenSSL with a proper KDF.
 */
std::string hashPassword(const std::string& password) {
    std::string hash = password;
    std::reverse(hash.begin(), hash.end());
    return hash + "_hashed";
}

// --- Core Logic ---
void setupDatabase() {
    userDatabase.clear();
    emailDatabase.clear();
    loggedInUsers.clear();

    std::vector<User> users = {
        {"alice", "alice@example.com", "password123"},
        {"bob", "bob@example.com", "bobpass"},
        {"charlie", "charlie@example.com", "securepass"}
    };

    for (auto& u : users) {
        std::string password = u.hashedPassword; // Store original password temporarily
        u.hashedPassword = hashPassword(password);
        userDatabase[u.username] = u;
        emailDatabase.insert(u.email);
    }
}

bool login(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end() && it->second.hashedPassword == hashPassword(password)) {
        loggedInUsers.insert(username);
        return true;
    }
    return false;
}

void logout(const std::string& username) {
    loggedInUsers.erase(username);
}

std::string changeEmail(const std::string& loggedInUsername, const std::string& oldEmail, const std::string& newEmail, const std::string& confirmPassword) {
    // 1. Check if user is logged in
    if (loggedInUsers.find(loggedInUsername) == loggedInUsers.end()) {
        return "Error: User is not logged in.";
    }

    // 2. Fetch user data
    auto it = userDatabase.find(loggedInUsername);
    if (it == userDatabase.end()) {
        return "Error: User data not found for logged-in user.";
    }
    User& user = it->second;

    // 3. Verify old email
    if (user.email != oldEmail) {
        return "Error: The provided old email does not match our records.";
    }

    // 4. Verify password
    if (user.hashedPassword != hashPassword(confirmPassword)) {
        return "Error: Incorrect password.";
    }

    // 5. Validate new email
    if (newEmail.empty() || newEmail.find('@') == std::string::npos) {
        return "Error: New email format is invalid.";
    }
    if (user.email == newEmail) {
        return "Error: New email cannot be the same as the old email.";
    }
    if (emailDatabase.count(newEmail)) {
        return "Error: New email is already in use by another account.";
    }

    // 6. All checks passed, perform the update
    emailDatabase.erase(user.email);
    user.email = newEmail;
    emailDatabase.insert(newEmail);
    
    return "Success: Email has been changed to " + newEmail;
}

void printUserInfo(const std::string& username) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end()) {
        std::cout << "User{username='" << it->second.username 
                  << "', email='" << it->second.email << "'}" << std::endl;
    } else {
        std::cout << "User " << username << " not found." << std::endl;
    }
}

int main() {
    // Test Case 1: Successful email change
    std::cout << "--- Test Case 1: Successful Change ---" << std::endl;
    setupDatabase();
    login("alice", "password123");
    std::cout << "Alice's current info: "; printUserInfo("alice");
    std::string result1 = changeEmail("alice", "alice@example.com", "alice_new@example.com", "password123");
    std::cout << "Result: " << result1 << std::endl;
    std::cout << "Alice's updated info: "; printUserInfo("alice");
    logout("alice");
    std::cout << std::endl;

    // Test Case 2: Failed change due to incorrect password
    std::cout << "--- Test Case 2: Incorrect Password ---" << std::endl;
    setupDatabase();
    login("bob", "bobpass");
    std::string result2 = changeEmail("bob", "bob@example.com", "bob_new@example.com", "wrongpassword");
    std::cout << "Result: " << result2 << std::endl;
    std::cout << "Bob's info (should be unchanged): "; printUserInfo("bob");
    logout("bob");
    std::cout << std::endl;
    
    // Test Case 3: Failed change because user is not logged in
    std::cout << "--- Test Case 3: User Not Logged In ---" << std::endl;
    setupDatabase();
    std::string result3 = changeEmail("charlie", "charlie@example.com", "charlie_new@example.com", "securepass");
    std::cout << "Result: " << result3 << std::endl;
    std::cout << std::endl;

    // Test Case 4: Failed change because new email is already taken
    std::cout << "--- Test Case 4: New Email Already Exists ---" << std::endl;
    setupDatabase();
    login("alice", "password123");
    std::string result4 = changeEmail("alice", "alice@example.com", "bob@example.com", "password123");
    std::cout << "Result: " << result4 << std::endl;
    logout("alice");
    std::cout << std::endl;

    // Test Case 5: Failed change due to incorrect old email provided
    std::cout << "--- Test Case 5: Incorrect Old Email Provided ---" << std::endl;
    setupDatabase();
    login("bob", "bobpass");
    std::string result5 = changeEmail("bob", "wrong_old@example.com", "bob_new@example.com", "bobpass");
    std::cout << "Result: " << result5 << std::endl;
    logout("bob");
    std::cout << std::endl;

    return 0;
}