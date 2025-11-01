
#include <iostream>
#include <string>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <regex>
#include <cstring>

class User {
private:
    std::string email;
    std::string passwordHash;
    std::string salt;
    bool isLoggedIn;

    std::string generateSalt() {
        unsigned char saltBytes[16];
        RAND_bytes(saltBytes, 16);
        
        std::stringstream ss;
        for(int i = 0; i < 16; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)saltBytes[i];
        }
        return ss.str();
    }

    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string combined = salt + password;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }

    bool isValidEmail(const std::string& email) {
        std::regex pattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        return std::regex_match(email, pattern);
    }

public:
    User(const std::string& email, const std::string& password) {
        this->email = email;
        this->salt = generateSalt();
        this->passwordHash = hashPassword(password, this->salt);
        this->isLoggedIn = false;
    }

    bool login(const std::string& email, const std::string& password) {
        if (this->email == email && this->passwordHash == hashPassword(password, this->salt)) {
            this->isLoggedIn = true;
            return true;
        }
        return false;
    }

    void logout() {
        this->isLoggedIn = false;
    }

    bool changeEmail(const std::string& oldEmail, const std::string& password, const std::string& newEmail) {
        if (!isLoggedIn) {
            std::cout << "Error: User must be logged in to change email" << std::endl;
            return false;
        }

        if (newEmail.empty() || !isValidEmail(newEmail)) {
            std::cout << "Error: Invalid new email format" << std::endl;
            return false;
        }

        if (this->email != oldEmail) {
            std::cout << "Error: Old email does not match" << std::endl;
            return false;
        }

        if (this->passwordHash != hashPassword(password, this->salt)) {
            std::cout << "Error: Incorrect password" << std::endl;
            return false;
        }

        this->email = newEmail;
        std::cout << "Email successfully changed to: " << newEmail << std::endl;
        return true;
    }

    std::string getEmail() const {
        return email;
    }

    bool isUserLoggedIn() const {
        return isLoggedIn;
    }
};

int main() {
    std::cout << "=== Test Case 1: Successful email change ===" << std::endl;
    User user1("user1@example.com", "password123");
    user1.login("user1@example.com", "password123");
    user1.changeEmail("user1@example.com", "password123", "newemail1@example.com");
    std::cout << "Current email: " << user1.getEmail() << "\\n" << std::endl;

    std::cout << "=== Test Case 2: Wrong old email ===" << std::endl;
    User user2("user2@example.com", "password456");
    user2.login("user2@example.com", "password456");
    user2.changeEmail("wrong@example.com", "password456", "newemail2@example.com");
    std::cout << "Current email: " << user2.getEmail() << "\\n" << std::endl;

    std::cout << "=== Test Case 3: Wrong password ===" << std::endl;
    User user3("user3@example.com", "password789");
    user3.login("user3@example.com", "password789");
    user3.changeEmail("user3@example.com", "wrongpassword", "newemail3@example.com");
    std::cout << "Current email: " << user3.getEmail() << "\\n" << std::endl;

    std::cout << "=== Test Case 4: Not logged in ===" << std::endl;
    User user4("user4@example.com", "password000");
    user4.changeEmail("user4@example.com", "password000", "newemail4@example.com");
    std::cout << "Current email: " << user4.getEmail() << "\\n" << std::endl;

    std::cout << "=== Test Case 5: Invalid new email format ===" << std::endl;
    User user5("user5@example.com", "password111");
    user5.login("user5@example.com", "password111");
    user5.changeEmail("user5@example.com", "password111", "invalidemail");
    std::cout << "Current email: " << user5.getEmail() << "\\n" << std::endl;

    return 0;
}
