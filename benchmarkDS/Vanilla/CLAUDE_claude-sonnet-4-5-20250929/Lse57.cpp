
#include <iostream>
#include <map>
#include <string>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

class Lse57 {
private:
    struct User {
        std::string email;
        std::string passwordHash;
        
        User() {}
        User(std::string e, std::string p) : email(e), passwordHash(hashPassword(p)) {}
    };
    
    std::map<std::string, User> users;
    std::string loggedInUser;
    
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    Lse57() : loggedInUser("") {}
    
    bool registerUser(const std::string& email, const std::string& password) {
        if (users.find(email) != users.end()) {
            return false;
        }
        users[email] = User(email, password);
        return true;
    }
    
    bool login(const std::string& email, const std::string& password) {
        auto it = users.find(email);
        if (it != users.end() && it->second.passwordHash == hashPassword(password)) {
            loggedInUser = email;
            return true;
        }
        return false;
    }
    
    bool changeEmail(const std::string& oldEmail, const std::string& newEmail, 
                     const std::string& confirmPassword) {
        // Check if user is logged in
        if (loggedInUser.empty()) {
            return false;
        }
        
        // Check if old email matches logged in user's email\n        if (loggedInUser != oldEmail) {\n            return false;\n        }\n        \n        // Check if old email exists\n        auto it = users.find(oldEmail);\n        if (it == users.end()) {\n            return false;\n        }\n        \n        // Verify confirm password\n        if (it->second.passwordHash != hashPassword(confirmPassword)) {\n            return false;\n        }\n        \n        // Check if new email already exists\n        if (users.find(newEmail) != users.end()) {\n            return false;\n        }\n        \n        // Change email\n        User user = it->second;\n        users.erase(it);\n        user.email = newEmail;\n        users[newEmail] = user;\n        loggedInUser = newEmail;\n        \n        return true;\n    }\n    \n    void logout() {\n        loggedInUser = "";\n    }\n};\n\nint main() {\n    // Test Case 1: Successful email change\n    std::cout << "Test Case 1: Successful email change" << std::endl;\n    Lse57 system1;\n    system1.registerUser("user1@example.com", "password123");\n    system1.login("user1@example.com", "password123");\n    bool result1 = system1.changeEmail("user1@example.com", "newuser1@example.com", "password123");\n    std::cout << "Result: " << (result1 ? "true" : "false") << " (Expected: true)" << std::endl;\n    system1.logout();\n    \n    // Test Case 2: Not logged in\n    std::cout << "\\nTest Case 2: Not logged in" << std::endl;\n    Lse57 system2;\n    system2.registerUser("user2@example.com", "password456");\n    bool result2 = system2.changeEmail("user2@example.com", "newuser2@example.com", "password456");\n    std::cout << "Result: " << (result2 ? "true" : "false") << " (Expected: false)" << std::endl;\n    \n    // Test Case 3: Wrong confirm password\n    std::cout << "\\nTest Case 3: Wrong confirm password" << std::endl;\n    Lse57 system3;\n    system3.registerUser("user3@example.com", "password789");\n    system3.login("user3@example.com", "password789");\n    bool result3 = system3.changeEmail("user3@example.com", "newuser3@example.com", "wrongpassword");\n    std::cout << "Result: " << (result3 ? "true" : "false") << " (Expected: false)" << std::endl;\n    system3.logout();\n    \n    // Test Case 4: Old email doesn't match logged in user
    std::cout << "\\nTest Case 4: Old email doesn't match logged in user" << std::endl;
    Lse57 system4;
    system4.registerUser("user4@example.com", "password111");
    system4.registerUser("user5@example.com", "password222");
    system4.login("user4@example.com", "password111");
    bool result4 = system4.changeEmail("user5@example.com", "newuser5@example.com", "password222");
    std::cout << "Result: " << (result4 ? "true" : "false") << " (Expected: false)" << std::endl;
    system4.logout();
    
    // Test Case 5: New email already exists
    std::cout << "\\nTest Case 5: New email already exists" << std::endl;
    Lse57 system5;
    system5.registerUser("user6@example.com", "password333");
    system5.registerUser("user7@example.com", "password444");
    system5.login("user6@example.com", "password333");
    bool result5 = system5.changeEmail("user6@example.com", "user7@example.com", "password333");
    std::cout << "Result: " << (result5 ? "true" : "false") << " (Expected: false)" << std::endl;
    system5.logout();
    
    return 0;
}
