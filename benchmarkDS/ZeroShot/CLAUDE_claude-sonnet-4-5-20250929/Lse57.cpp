
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>

class User {
private:
    std::string email;
    std::string passwordHash;
    
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
    User() {}
    
    User(const std::string& email, const std::string& password) {
        this->email = email;
        this->passwordHash = hashPassword(password);
    }
    
    std::string getEmail() const {
        return email;
    }
    
    void setEmail(const std::string& newEmail) {
        email = newEmail;
    }
    
    bool verifyPassword(const std::string& password) {
        return passwordHash == hashPassword(password);
    }
};

class Lse57 {
private:
    std::map<std::string, User> userDatabase;
    User* loggedInUser;
    bool isLoggedIn;
    
    bool isValidEmail(const std::string& email) {
        const std::regex pattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        return std::regex_match(email, pattern);
    }
    
public:
    Lse57() : loggedInUser(nullptr), isLoggedIn(false) {}
    
    bool registerUser(const std::string& email, const std::string& password) {
        if (userDatabase.find(email) == userDatabase.end()) {
            User newUser(email, password);
            userDatabase[email] = newUser;
            return true;
        }
        return false;
    }
    
    bool login(const std::string& email, const std::string& password) {
        auto it = userDatabase.find(email);
        if (it != userDatabase.end()) {
            if (it->second.verifyPassword(password)) {
                loggedInUser = &(it->second);
                isLoggedIn = true;
                return true;
            }
        }
        return false;
    }
    
    void logout() {
        loggedInUser = nullptr;
        isLoggedIn = false;
    }
    
    std::string changeEmail(const std::string& oldEmail, const std::string& newEmail, 
                           const std::string& confirmPassword) {
        // Check if user is logged in
        if (!isLoggedIn || loggedInUser == nullptr) {
            return "Error: User not logged in";
        }
        
        // Verify old email matches logged in user's email
        if (loggedInUser->getEmail() != oldEmail) {
            return "Error: Old email does not match current user email";
        }
        
        // Verify old email exists in database
        if (userDatabase.find(oldEmail) == userDatabase.end()) {
            return "Error: Old email does not exist";
        }
        
        // Verify confirm password is correct
        if (!loggedInUser->verifyPassword(confirmPassword)) {
            return "Error: Incorrect password";
        }
        
        // Check if new email is already in use
        if (userDatabase.find(newEmail) != userDatabase.end() && newEmail != oldEmail) {
            return "Error: New email already exists";
        }
        
        // Validate email format
        if (!isValidEmail(newEmail)) {
            return "Error: Invalid email format";
        }
        
        // Change email
        User updatedUser = userDatabase[oldEmail];
        userDatabase.erase(oldEmail);
        updatedUser.setEmail(newEmail);
        userDatabase[newEmail] = updatedUser;
        loggedInUser = &userDatabase[newEmail];
        
        return "Success: Email changed successfully";
    }
};

int main() {
    Lse57 system;
    
    // Test Case 1: Successful email change
    std::cout << "Test Case 1: Successful email change" << std::endl;
    system.registerUser("user1@example.com", "password123");
    system.login("user1@example.com", "password123");
    std::string result1 = system.changeEmail("user1@example.com", "newuser1@example.com", "password123");
    std::cout << result1 << std::endl << std::endl;
    system.logout();
    
    // Test Case 2: User not logged in
    std::cout << "Test Case 2: User not logged in" << std::endl;
    std::string result2 = system.changeEmail("user2@example.com", "newuser2@example.com", "password123");
    std::cout << result2 << std::endl << std::endl;
    
    // Test Case 3: Wrong password
    std::cout << "Test Case 3: Wrong password" << std::endl;
    system.registerUser("user3@example.com", "password456");
    system.login("user3@example.com", "password456");
    std::string result3 = system.changeEmail("user3@example.com", "newuser3@example.com", "wrongpassword");
    std::cout << result3 << std::endl << std::endl;
    system.logout();
    
    // Test Case 4: Old email mismatch
    std::cout << "Test Case 4: Old email mismatch" << std::endl;
    system.registerUser("user4@example.com", "password789");
    system.login("user4@example.com", "password789");
    std::string result4 = system.changeEmail("different@example.com", "newuser4@example.com", "password789");
    std::cout << result4 << std::endl << std::endl;
    system.logout();
    
    // Test Case 5: New email already exists
    std::cout << "Test Case 5: New email already exists" << std::endl;
    system.registerUser("user5@example.com", "password101");
    system.registerUser("existing@example.com", "password102");
    system.login("user5@example.com", "password101");
    std::string result5 = system.changeEmail("user5@example.com", "existing@example.com", "password101");
    std::cout << result5 << std::endl;
    system.logout();
    
    return 0;
}
