
#include <iostream>
#include <string>
#include <map>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

class Lse56 {
private:
    struct UserAccount {
        std::string email;
        std::string passwordHash;
    };
    
    std::map<std::string, UserAccount> userDatabase;
    std::string currentLoggedInUser;
    
    std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    Lse56() : currentLoggedInUser("") {}
    
    bool registerUser(const std::string& email, const std::string& password) {
        if (userDatabase.find(email) == userDatabase.end()) {
            UserAccount user;
            user.email = email;
            user.passwordHash = hashPassword(password);
            userDatabase[email] = user;
            return true;
        }
        return false;
    }
    
    bool login(const std::string& email, const std::string& password) {
        auto it = userDatabase.find(email);
        if (it != userDatabase.end() && it->second.passwordHash == hashPassword(password)) {
            currentLoggedInUser = email;
            return true;
        }
        return false;
    }
    
    void logout() {
        currentLoggedInUser = "";
    }
    
    bool changeEmail(const std::string& oldEmail, const std::string& newEmail, 
                     const std::string& confirmPassword) {
        // Check if user is logged in
        if (currentLoggedInUser.empty()) {
            std::cout << "Error: User must be logged in to change email" << std::endl;
            return false;
        }
        
        // Check if old email matches current user's email\n        auto it = userDatabase.find(currentLoggedInUser);\n        if (it->second.email != oldEmail) {\n            std::cout << "Error: Old email does not match current email" << std::endl;\n            return false;\n        }\n        \n        // Check if new email is different from old email\n        if (oldEmail == newEmail) {\n            std::cout << "Error: New email must be different from old email" << std::endl;\n            return false;\n        }\n        \n        // Verify password\n        if (it->second.passwordHash != hashPassword(confirmPassword)) {\n            std::cout << "Error: Incorrect password" << std::endl;\n            return false;\n        }\n        \n        // Check if new email is already taken\n        if (userDatabase.find(newEmail) != userDatabase.end()) {\n            std::cout << "Error: Email already exists" << std::endl;\n            return false;\n        }\n        \n        // Change email\n        UserAccount user = it->second;\n        userDatabase.erase(it);\n        user.email = newEmail;\n        userDatabase[newEmail] = user;\n        currentLoggedInUser = newEmail;\n        \n        std::cout << "Email changed successfully from " << oldEmail << " to " << newEmail << std::endl;\n        return true;\n    }\n};\n\nint main() {\n    Lse56 system;\n    \n    // Test Case 1: Register users and successful email change\n    system.registerUser("user1@example.com", "password123");\n    system.login("user1@example.com", "password123");\n    std::cout << "Test 1: " << system.changeEmail("user1@example.com", "newuser1@example.com", "password123") << std::endl;\n    system.logout();\n    \n    // Test Case 2: Attempt to change email without being logged in\n    std::cout << "\\nTest 2: " << system.changeEmail("newuser1@example.com", "another@example.com", "password123") << std::endl;\n    \n    // Test Case 3: Incorrect password\n    system.registerUser("user2@example.com", "pass456");\n    system.login("user2@example.com", "pass456");\n    std::cout << "\\nTest 3: " << system.changeEmail("user2@example.com", "newuser2@example.com", "wrongpassword") << std::endl;\n    system.logout();\n    \n    // Test Case 4: Old email doesn't match current email
    system.login("user2@example.com", "pass456");
    std::cout << "\\nTest 4: " << system.changeEmail("wrongemail@example.com", "newuser2@example.com", "pass456") << std::endl;
    system.logout();
    
    // Test Case 5: New email same as old email
    system.login("user2@example.com", "pass456");
    std::cout << "\\nTest 5: " << system.changeEmail("user2@example.com", "user2@example.com", "pass456") << std::endl;
    system.logout();
    
    return 0;
}
