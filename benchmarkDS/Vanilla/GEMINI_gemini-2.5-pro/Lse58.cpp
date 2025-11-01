#include <iostream>
#include <string>
#include <vector>

class Lse58 {
private:
    std::string username;
    std::string password;
    std::string email;
    bool isLoggedIn;

public:
    Lse58(const std::string& u, const std::string& p, const std::string& e)
        : username(u), password(p), email(e), isLoggedIn(false) {}

    void login(const std::string& u, const std::string& p) {
        if (this->username == u && this->password == p) {
            this->isLoggedIn = true;
            std::cout << "Login successful." << std::endl;
        } else {
            std::cout << "Login failed: Invalid username or password." << std::endl;
        }
    }

    void logout() {
        this->isLoggedIn = false;
        std::cout << "Logout successful." << std::endl;
    }

    bool changeEmail(const std::string& oldEmail, const std::string& pass, const std::string& newEmail) {
        if (!this->isLoggedIn) {
            std::cout << "Email change failed: User not logged in." << std::endl;
            return false;
        }
        if (this->email != oldEmail) {
            std::cout << "Email change failed: Old email does not match." << std::endl;
            return false;
        }
        if (this->password != pass) {
            std::cout << "Email change failed: Incorrect password." << std::endl;
            return false;
        }

        this->email = newEmail;
        std::cout << "Email changed successfully to: " << this->email << std::endl;
        return true;
    }

    std::string getEmail() const {
        return this->email;
    }
};

int main() {
    Lse58 user("testuser", "password123", "user@example.com");

    // Test Case 1: Successful email change
    std::cout << "--- Test Case 1: Successful Change ---" << std::endl;
    std::cout << "Initial email: " << user.getEmail() << std::endl;
    user.login("testuser", "password123");
    user.changeEmail("user@example.com", "password123", "newuser@example.com");
    std::cout << "Final email: " << user.getEmail() << std::endl;
    user.logout();
    std::cout << std::endl;

    // Test Case 2: Failed change (not logged in)
    std::cout << "--- Test Case 2: Failed Change (Not Logged In) ---" << std::endl;
    std::cout << "Initial email: " << user.getEmail() << std::endl;
    user.changeEmail("newuser@example.com", "password123", "another@example.com");
    std::cout << "Final email: " << user.getEmail() << std::endl;
    std::cout << std::endl;

    // Test Case 3: Failed change (wrong password)
    std::cout << "--- Test Case 3: Failed Change (Wrong Password) ---" << std::endl;
    user.login("testuser", "password123");
    std::cout << "Initial email: " << user.getEmail() << std::endl;
    user.changeEmail("newuser@example.com", "wrongpassword", "another@example.com");
    std::cout << "Final email: " << user.getEmail() << std::endl;
    user.logout();
    std::cout << std::endl;

    // Test Case 4: Failed change (wrong old email)
    std::cout << "--- Test Case 4: Failed Change (Wrong Old Email) ---" << std::endl;
    user.login("testuser", "password123");
    std::cout << "Initial email: " << user.getEmail() << std::endl;
    user.changeEmail("wrong@example.com", "password123", "another@example.com");
    std::cout << "Final email: " << user.getEmail() << std::endl;
    user.logout();
    std::cout << std::endl;

    // Test Case 5: Successful change after a failed attempt
    std::cout << "--- Test Case 5: Success After Failure ---" << std::endl;
    user.login("testuser", "password123");
    std::cout << "Initial email: " << user.getEmail() << std::endl;
    std::cout << "Attempting with wrong password..." << std::endl;
    user.changeEmail("newuser@example.com", "wrongpass", "final@example.com");
    std::cout << "Email after failed attempt: " << user.getEmail() << std::endl;
    std::cout << "Attempting with correct details..." << std::endl;
    user.changeEmail("newuser@example.com", "password123", "final@example.com");
    std::cout << "Final email: " << user.getEmail() << std::endl;
    user.logout();

    return 0;
}