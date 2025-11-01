
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <regex>
#include <openssl/sha.h>

class User {
private:
    std::string username;
    std::string email;
    std::string passwordHash;
    bool loggedIn;

    std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
               password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(hash[i]);
        }
        return ss.str();
    }

    bool isValidEmail(const std::string& email) {
        std::regex pattern(R"(^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$)");
        return std::regex_match(email, pattern);
    }

public:
    User(const std::string& username, const std::string& email, 
         const std::string& password) 
        : username(username), email(email), loggedIn(false) {
        passwordHash = hashPassword(password);
    }

    bool login(const std::string& password) {
        if (hashPassword(password) == passwordHash) {
            loggedIn = true;
            return true;
        }
        return false;
    }

    void logout() {
        loggedIn = false;
    }

    bool changeEmail(const std::string& oldEmail, const std::string& password, 
                     const std::string& newEmail) {
        if (!loggedIn) {
            return false;
        }
        if (email != oldEmail) {
            return false;
        }
        if (hashPassword(password) != passwordHash) {
            return false;
        }
        if (newEmail.empty() || !isValidEmail(newEmail)) {
            return false;
        }
        email = newEmail;
        return true;
    }

    std::string getEmail() const {
        return email;
    }

    std::string getUsername() const {
        return username;
    }

    bool isLoggedInStatus() const {
        return loggedIn;
    }
};

int main() {
    // Test Case 1: Successful email change
    User user1("john_doe", "john@example.com", "password123");
    user1.login("password123");
    bool result1 = user1.changeEmail("john@example.com", "password123", 
                                     "newemail@example.com");
    std::cout << "Test 1 - Successful change: " << result1 
              << ", New email: " << user1.getEmail() << std::endl;

    // Test Case 2: Wrong old email
    User user2("jane_doe", "jane@example.com", "securepass");
    user2.login("securepass");
    bool result2 = user2.changeEmail("wrong@example.com", "securepass", 
                                     "newemail@example.com");
    std::cout << "Test 2 - Wrong old email: " << result2 
              << ", Email unchanged: " << user2.getEmail() << std::endl;

    // Test Case 3: Wrong password
    User user3("bob_smith", "bob@example.com", "mypassword");
    user3.login("mypassword");
    bool result3 = user3.changeEmail("bob@example.com", "wrongpassword", 
                                     "newemail@example.com");
    std::cout << "Test 3 - Wrong password: " << result3 
              << ", Email unchanged: " << user3.getEmail() << std::endl;

    // Test Case 4: Not logged in
    User user4("alice_wonder", "alice@example.com", "alicepass");
    bool result4 = user4.changeEmail("alice@example.com", "alicepass", 
                                     "newemail@example.com");
    std::cout << "Test 4 - Not logged in: " << result4 
              << ", Email unchanged: " << user4.getEmail() << std::endl;

    // Test Case 5: Invalid new email format
    User user5("charlie_brown", "charlie@example.com", "charpass");
    user5.login("charpass");
    bool result5 = user5.changeEmail("charlie@example.com", "charpass", 
                                     "invalidemail");
    std::cout << "Test 5 - Invalid email format: " << result5 
              << ", Email unchanged: " << user5.getEmail() << std::endl;

    return 0;
}
