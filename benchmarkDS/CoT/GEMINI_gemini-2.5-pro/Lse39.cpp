#include <iostream>
#include <string>

/**
 * @brief Checks if the provided username and password are correct.
 * NOTE: Hardcoding credentials and using plaintext passwords are insecure
 * and done here for demonstration purposes only.
 * 
 * @param username The username to check.
 * @param password The password to check.
 * @return A string indicating success or failure.
 */
std::string login(const std::string& username, const std::string& password) {
    const std::string CORRECT_USERNAME = "admin";
    const std::string CORRECT_PASSWORD = "password123";
    
    // In a real application, you would use a constant-time comparison
    // to prevent timing attacks.
    if (username == CORRECT_USERNAME && password == CORRECT_PASSWORD) {
        return "Login successful! Redirecting to the session page.";
    } else {
        return "Incorrect username or password.";
    }
}

int main() {
    std::cout << "--- Login System Test Cases ---" << std::endl;

    // Test Case 1: Correct credentials
    std::cout << "\nTest Case 1 (Correct):" << std::endl;
    std::cout << "Input: user='admin', pass='password123'" << std::endl;
    std::cout << "Output: " << login("admin", "password123") << std::endl;

    // Test Case 2: Incorrect password
    std::cout << "\nTest Case 2 (Incorrect Password):" << std::endl;
    std::cout << "Input: user='admin', pass='wrongpassword'" << std::endl;
    std::cout << "Output: " << login("admin", "wrongpassword") << std::endl;

    // Test Case 3: Incorrect username
    std::cout << "\nTest Case 3 (Incorrect Username):" << std::endl;
    std::cout << "Input: user='user', pass='password123'" << std::endl;
    std::cout << "Output: " << login("user", "password123") << std::endl;

    // Test Case 4: Both incorrect
    std::cout << "\nTest Case 4 (Both Incorrect):" << std::endl;
    std::cout << "Input: user='user', pass='wrongpassword'" << std::endl;
    std::cout << "Output: " << login("user", "wrongpassword") << std::endl;

    // Test Case 5: Case-sensitive check
    std::cout << "\nTest Case 5 (Case-Sensitive):" << std::endl;
    std::cout << "Input: user='Admin', pass='password123'" << std::endl;
    std::cout << "Output: " << login("Admin", "password123") << std::endl;

    std::cout << "\n--- End of Test Cases ---" << std::endl;

    return 0;
}