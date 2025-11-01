#include <iostream>
#include <string>

class Lse58 {
private:
    std::string username;
    std::string password; // In a real app, this should be a hashed password
    std::string email;
    bool isLoggedIn;

public:
    Lse58(const std::string& user, const std::string& pass, const std::string& mail)
        : username(user), password(pass), email(mail), isLoggedIn(false) {}

    bool login(const std::string& user, const std::string& pass) {
        if (this->username == user && this->password == pass) {
            this->isLoggedIn = true;
            std::cout << "Login successful for user: " << this->username << std::endl;
            return true;
        }
        std::cout << "Login failed for user: " << user << std::endl;
        return false;
    }

    void logout() {
        this->isLoggedIn = false;
        std::cout << this->username << " has been logged out." << std::endl;
    }

    bool changeEmail(const std::string& oldEmail, const std::string& confirmPassword, const std::string& newEmail) {
        if (!this->isLoggedIn) {
            std::cout << "Error: You must be logged in to change your email." << std::endl;
            return false;
        }

        if (this->email == oldEmail && this->password == confirmPassword) {
            this->email = newEmail;
            std::cout << "Success: Email has been changed to " << newEmail << std::endl;
            return true;
        } else {
            std::cout << "Error: Incorrect old email or password. Email not changed." << std::endl;
            return false;
        }
    }

    std::string getEmail() const {
        return this->email;
    }
};

int main() {
    Lse58 user("testuser", "password123", "user@example.com");

    std::cout << "--- Test Case 1: Successful Email Change ---" << std::endl;
    user.login("testuser", "password123");
    user.changeEmail("user@example.com", "password123", "newuser@example.com");
    std::cout << "Current email: " << user.getEmail() << std::endl;
    user.logout();
    std::cout << std::endl;

    // Reset user for next tests
    user = Lse58("testuser", "password123", "user@example.com");

    std::cout << "--- Test Case 2: Failed Change (Not Logged In) ---" << std::endl;
    user.changeEmail("user@example.com", "password123", "another@example.com");
    std::cout << "Current email: " << user.getEmail() << std::endl;
    std::cout << std::endl;

    std::cout << "--- Test Case 3: Failed Change (Wrong Password) ---" << std::endl;
    user.login("testuser", "password123");
    user.changeEmail("user@example.com", "wrongpassword", "another@example.com");
    std::cout << "Current email: " << user.getEmail() << std::endl;
    user.logout();
    std::cout << std::endl;
    
    // Reset user
    user = Lse58("testuser", "password123", "user@example.com");

    std::cout << "--- Test Case 4: Failed Change (Wrong Old Email) ---" << std::endl;
    user.login("testuser", "password123");
    user.changeEmail("wrong@example.com", "password123", "another@example.com");
    std::cout << "Current email: " << user.getEmail() << std::endl;
    
    std::cout << "--- Test Case 5: Failed Change (Logged out then attempt) ---" << std::endl;
    user.logout(); // Ensuring user is logged out
    user.changeEmail("user@example.com", "password123", "final@example.com");
    std::cout << "Current email: " << user.getEmail() << std::endl;

    return 0;
}