#include <iostream>
#include <string>
#include <vector>

// In a real application, never store passwords in plain text.
// Always use a strong, salted hashing algorithm like Argon2, scrypt, or bcrypt.
struct User {
    std::string email;
    std::string password; // Plain text for demonstration only.
    bool isLoggedIn;

    User(const std::string& email, const std::string& password)
        : email(email), password(password), isLoggedIn(false) {}

    void login(const std::string& pwd) {
        if (this->password == pwd) {
            this->isLoggedIn = true;
            std::cout << "User " << this->email << " logged in successfully." << std::endl;
        } else {
            std::cout << "Login failed for user " << this->email << std::endl;
        }
    }

    void logout() {
        this->isLoggedIn = false;
        std::cout << "User " << this->email << " logged out." << std::endl;
    }
};

class Lse56 {
public:
    /**
     * Attempts to change a user's email address after performing security checks.
     *
     * @param user The user object attempting the change (passed by reference).
     * @param oldEmail The user's claimed old email address.
     * @param newEmail The desired new email address.
     * @param password The user's password for confirmation.
     * @return A string indicating success or the specific reason for failure.
     */
    static std::string changeEmail(User& user, const std::string& oldEmail, const std::string& newEmail, const std::string& password) {
        // 1. Check if the user is logged in
        if (!user.isLoggedIn) {
            return "Error: User is not logged in.";
        }

        // 2. Check if the provided old email matches the current one
        if (user.email != oldEmail) {
            return "Error: Old email does not match the current email.";
        }

        // 3. Check if the provided password is correct
        if (user.password != password) {
            return "Error: Incorrect password.";
        }

        // 4. Check if the new email is different from the old email
        if (user.email == newEmail) {
            return "Error: New email cannot be the same as the old email.";
        }
        
        // In a real application, you would also validate the new email format.

        // All checks passed, update the email
        user.email = newEmail;
        return "Success: Email has been changed to " + newEmail;
    }
};

int main() {
    // --- Test Cases ---
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Successful Email Change
    std::cout << "\n--- Test Case 1: Successful Change ---" << std::endl;
    User user1("test1@example.com", "password123");
    user1.login("password123");
    std::string result1 = Lse56::changeEmail(user1, "test1@example.com", "new1@example.com", "password123");
    std::cout << "Result: " << result1 << std::endl;
    std::cout << "User's current email: " << user1.email << std::endl;
    user1.logout();

    // Test Case 2: User Not Logged In
    std::cout << "\n--- Test Case 2: User Not Logged In ---" << std::endl;
    User user2("test2@example.com", "password456");
    // user2 does not log in
    std::string result2 = Lse56::changeEmail(user2, "test2@example.com", "new2@example.com", "password456");
    std::cout << "Result: " << result2 << std::endl;
    std::cout << "User's current email: " << user2.email << std::endl;

    // Test Case 3: Incorrect Old Email
    std::cout << "\n--- Test Case 3: Incorrect Old Email ---" << std::endl;
    User user3("test3@example.com", "password789");
    user3.login("password789");
    std::string result3 = Lse56::changeEmail(user3, "wrong-email@example.com", "new3@example.com", "password789");
    std::cout << "Result: " << result3 << std::endl;
    std::cout << "User's current email: " << user3.email << std::endl;
    user3.logout();

    // Test Case 4: Incorrect Password
    std::cout << "\n--- Test Case 4: Incorrect Password ---" << std::endl;
    User user4("test4@example.com", "securepass");
    user4.login("securepass");
    std::string result4 = Lse56::changeEmail(user4, "test4@example.com", "new4@example.com", "wrongpass");
    std::cout << "Result: " << result4 << std::endl;
    std::cout << "User's current email: " << user4.email << std::endl;
    user4.logout();

    // Test Case 5: New Email is Same as Old Email
    std::cout << "\n--- Test Case 5: New Email is Same as Old Email ---" << std::endl;
    User user5("test5@example.com", "pass12345");
    user5.login("pass12345");
    std::string result5 = Lse56::changeEmail(user5, "test5@example.com", "test5@example.com", "pass12345");
    std::cout << "Result: " << result5 << std::endl;
    std::cout << "User's current email: " << user5.email << std::endl;
    user5.logout();

    return 0;
}