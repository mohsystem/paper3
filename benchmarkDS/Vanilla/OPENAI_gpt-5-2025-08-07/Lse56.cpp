#include <iostream>
#include <string>

struct User {
    std::string email;
    std::string password;
    User(const std::string& e, const std::string& p) : email(e), password(p) {}
};

std::string changeEmail(User& user, bool loggedIn, const std::string& oldEmail, const std::string& newEmail, const std::string& confirmPassword) {
    if (oldEmail.empty() || newEmail.empty() || confirmPassword.empty()) {
        return "ERROR: Invalid input.";
    }
    if (!loggedIn) {
        return "ERROR: User must be logged in.";
    }
    if (oldEmail != user.email) {
        return "ERROR: Old email does not match.";
    }
    if (newEmail == user.email) {
        return "ERROR: New email must be different from old email.";
    }
    if (confirmPassword != user.password) {
        return "ERROR: Incorrect password.";
    }
    user.email = newEmail;
    return "SUCCESS: Email changed to " + newEmail;
}

int main() {
    // Test case 1: Success
    User u1("user@example.com", "secret123");
    std::cout << changeEmail(u1, true, "user@example.com", "new@example.com", "secret123") << std::endl;

    // Test case 2: Not logged in
    User u2("user@example.com", "secret123");
    std::cout << changeEmail(u2, false, "user@example.com", "newer@example.com", "secret123") << std::endl;

    // Test case 3: Wrong old email
    User u3("user@example.com", "secret123");
    std::cout << changeEmail(u3, true, "wrong@example.com", "newer@example.com", "secret123") << std::endl;

    // Test case 4: Incorrect password
    User u4("user@example.com", "secret123");
    std::cout << changeEmail(u4, true, "user@example.com", "newer@example.com", "badpass") << std::endl;

    // Test case 5: New email same as old
    User u5("user@example.com", "secret123");
    std::cout << changeEmail(u5, true, "user@example.com", "user@example.com", "secret123") << std::endl;

    return 0;
}