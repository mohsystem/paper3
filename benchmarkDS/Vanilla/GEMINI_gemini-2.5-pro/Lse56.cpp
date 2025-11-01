#include <iostream>
#include <string>
#include <vector>

// Simulate a single user's session data
std::string currentUserEmail = "";
std::string currentUserPassword = "";
bool isLoggedIn = false;

/**
 * @brief Helper to set up an initial user for testing.
 * @param email The user's email.
 * @param password The user's password.
 */
void registerUser(const std::string& email, const std::string& password) {
    currentUserEmail = email;
    currentUserPassword = password;
    isLoggedIn = false; // User is registered, not logged in
}

/**
 * @brief Logs a user in.
 * @param email The email to log in with.
 * @param password The password for the account.
 * @return true if login is successful, false otherwise.
 */
bool login(const std::string& email, const std::string& password) {
    if (!currentUserEmail.empty() && currentUserEmail == email &&
        !currentUserPassword.empty() && currentUserPassword == password) {
        isLoggedIn = true;
        return true;
    }
    isLoggedIn = false;
    return false;
}

/**
 * @brief Logs the current user out.
 */
void logout() {
    isLoggedIn = false;
}

/**
 * @brief Changes the current user's email after validation.
 * @param oldEmail The user's current email for confirmation.
 * @param newEmail The desired new email.
 * @param password The user's password for confirmation.
 * @return A std::string message indicating success or the specific failure reason.
 */
std::string changeEmail(const std::string& oldEmail, const std::string& newEmail, const std::string& password) {
    if (!isLoggedIn) {
        return "Error: User is not logged in.";
    }
    if (currentUserEmail != oldEmail) {
        return "Error: Old email does not match the current email.";
    }
    if (currentUserPassword != password) {
        return "Error: Incorrect password.";
    }
    if (oldEmail == newEmail) {
        return "Error: New email cannot be the same as the old email.";
    }

    // If all checks pass, change the email
    currentUserEmail = newEmail;
    return "Email changed successfully to " + newEmail;
}

int main() {
    // Initial setup
    std::string initialEmail = "user@example.com";
    std::string initialPassword = "password123";

    // --- Test Case 1: Successful Email Change ---
    std::cout << "--- Test Case 1: Successful Change ---" << std::endl;
    registerUser(initialEmail, initialPassword);
    std::cout << "User registered: " << initialEmail << std::endl;
    bool loginStatus = login(initialEmail, initialPassword);
    std::cout << "Login status: " << std::boolalpha << loginStatus << std::endl;
    std::string result1 = changeEmail(initialEmail, "new.user@example.com", initialPassword);
    std::cout << "Change email result: " << result1 << std::endl;
    std::cout << "Current email is now: " << currentUserEmail << std::endl << std::endl;

    // --- Test Case 2: Failed Change (Incorrect Password) ---
    std::cout << "--- Test Case 2: Incorrect Password ---" << std::endl;
    registerUser(initialEmail, initialPassword); // Reset state
    std::cout << "User registered: " << initialEmail << std::endl;
    login(initialEmail, initialPassword);
    std::cout << "Login status: " << std::boolalpha << isLoggedIn << std::endl;
    std::string result2 = changeEmail(initialEmail, "another.user@example.com", "wrongpassword");
    std::cout << "Change email result: " << result2 << std::endl;
    std::cout << "Current email is still: " << currentUserEmail << std::endl << std::endl;

    // --- Test Case 3: Failed Change (Not Logged In) ---
    std::cout << "--- Test Case 3: Not Logged In ---" << std::endl;
    registerUser(initialEmail, initialPassword);
    std::cout << "User registered: " << initialEmail << std::endl;
    logout(); // Ensure user is logged out
    std::cout << "Login status: " << std::boolalpha << isLoggedIn << std::endl;
    std::string result3 = changeEmail(initialEmail, "new.user@example.com", initialPassword);
    std::cout << "Change email result: " << result3 << std::endl << std::endl;

    // --- Test Case 4: Failed Change (New Email is Same as Old) ---
    std::cout << "--- Test Case 4: New Email is Same as Old ---" << std::endl;
    registerUser(initialEmail, initialPassword);
    std::cout << "User registered: " << initialEmail << std::endl;
    login(initialEmail, initialPassword);
    std::cout << "Login status: " << std::boolalpha << isLoggedIn << std::endl;
    std::string result4 = changeEmail(initialEmail, initialEmail, initialPassword);
    std::cout << "Change email result: " << result4 << std::endl << std::endl;

    // --- Test Case 5: Failed Change (Incorrect Old Email Provided) ---
    std::cout << "--- Test Case 5: Incorrect Old Email Provided ---" << std::endl;
    registerUser(initialEmail, initialPassword);
    std::cout << "User registered: " << initialEmail << std::endl;
    login(initialEmail, initialPassword);
    std::cout << "Login status: " << std::boolalpha << isLoggedIn << std::endl;
    std::string result5 = changeEmail("wrong.old@example.com", "new.user@example.com", initialPassword);
    std::cout << "Change email result: " << result5 << std::endl << std::endl;

    return 0;
}