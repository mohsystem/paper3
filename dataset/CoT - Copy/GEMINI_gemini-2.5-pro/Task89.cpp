#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio> // For std::remove

class Task89 {
public:
    /**
     * WARNING: Storing passwords in plaintext is extremely insecure.
     * In a real-world application, you should always use a strong, salted,
     * one-way hashing algorithm like Argon2, scrypt, or bcrypt to store password hashes,
     * not the passwords themselves. This implementation is for demonstration purposes only.
     *
     * Compares a user-provided password with one stored in a file using a
     * constant-time comparison algorithm to mitigate timing attacks.
     *
     * @param filename The path to the file containing the stored password.
     * @param userInput The password entered by the user.
     * @return true if the passwords match, false otherwise.
     */
    static bool comparePassword(const std::string& filename, const std::string& userInput) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open password file." << std::endl;
            return false;
        }

        std::string storedPassword;
        // Read the first line from the file
        std::getline(file, storedPassword);
        file.close(); // RAII will close it, but explicit is fine

        // Length check is performed first. This can leak password length information.
        if (storedPassword.length() != userInput.length()) {
            return false;
        }

        // Constant-time comparison to prevent timing attacks.
        int diff = 0;
        for (size_t i = 0; i < storedPassword.length(); ++i) {
            // XOR the characters and OR the result into diff.
            diff |= storedPassword[i] ^ userInput[i];
        }

        // If diff is 0, all characters matched.
        return diff == 0;
    }
};

int main() {
    const std::string testFilename = "password.txt";
    const std::string correctPassword = "S3cur3P@ssw0rd!";

    // Setup: Create a dummy password file for testing
    std::ofstream outFile(testFilename);
    if (!outFile) {
        std::cerr << "Failed to create test password file." << std::endl;
        return 1;
    }
    outFile << correctPassword;
    outFile.close();

    std::cout << "--- C++ Test Cases ---" << std::endl;
    // Test Case 1: Correct password
    bool result1 = Task89::comparePassword(testFilename, "S3cur3P@ssw0rd!");
    std::cout << "Test 1 (Correct): " << (result1 ? "PASS" : "FAIL") << std::endl;

    // Test Case 2: Incorrect password (different first character)
    bool result2 = Task89::comparePassword(testFilename, "X3cur3P@ssw0rd!");
    std::cout << "Test 2 (Incorrect, first char): " << (!result2 ? "PASS" : "FAIL") << std::endl;

    // Test Case 3: Incorrect password (different last character)
    bool result3 = Task89::comparePassword(testFilename, "S3cur3P@ssw0rd?");
    std::cout << "Test 3 (Incorrect, last char): " << (!result3 ? "PASS" : "FAIL") << std::endl;

    // Test Case 4: Incorrect password (shorter)
    bool result4 = Task89::comparePassword(testFilename, "S3cur3");
    std::cout << "Test 4 (Incorrect, shorter): " << (!result4 ? "PASS" : "FAIL") << std::endl;

    // Test Case 5: Incorrect password (longer)
    bool result5 = Task89::comparePassword(testFilename, "S3cur3P@ssw0rd!Extra");
    std::cout << "Test 5 (Incorrect, longer): " << (!result5 ? "PASS" : "FAIL") << std::endl;

    // Cleanup: Delete the test file
    std::remove(testFilename.c_str());

    return 0;
}