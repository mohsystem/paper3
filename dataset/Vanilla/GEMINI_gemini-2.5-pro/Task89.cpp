#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio> // For remove()

/**
 * @brief Creates a dummy file with the given password.
 * @param filename The name of the file to create.
 * @param password The password to store in the file.
 */
void createPasswordFile(const std::string& filename, const std::string& password) {
    std::ofstream file(filename);
    if (file.is_open()) {
        file << password;
        file.close();
    } else {
        std::cerr << "Error creating password file." << std::endl;
    }
}

/**
 * @brief Compares the password in a file with the user-provided password
 * by checking each character sequentially.
 * @param filename The name of the file containing the stored password.
 * @param userInput The password entered by the user.
 * @return true if the passwords match, false otherwise.
 */
bool comparePassword(const std::string& filename, const std::string& userInput) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error reading password file." << std::endl;
        return false;
    }

    std::string storedPassword;
    std::getline(file, storedPassword); // Read the entire line
    file.close();

    // First, check if the lengths are the same.
    if (storedPassword.length() != userInput.length()) {
        return false;
    }

    // Compare each character sequentially
    for (size_t i = 0; i < storedPassword.length(); ++i) {
        if (storedPassword[i] != userInput[i]) {
            return false; // Mismatch found
        }
    }

    // If the loop completes, all characters matched
    return true;
}

int main() {
    const std::string filename = "password.txt";
    const std::string correctPassword = "P@ssw0rd123";

    // Create the password file for testing
    createPasswordFile(filename, correctPassword);

    // --- Test Cases ---
    std::vector<std::string> testPasswords = {
        "P@ssw0rd123",    // 1. Correct password
        "p@ssw0rd123",    // 2. Incorrect case
        "P@ssw0rd",       // 3. Incorrect length (shorter)
        "P@ssw0rd124",    // 4. Same length, different character
        ""                // 5. Empty password
    };
    
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& test : testPasswords) {
        bool isMatch = comparePassword(filename, test);
        std::cout << "Testing with \"" << test << "\": " << (isMatch ? "Match" : "No Match") << std::endl;
    }

    // Clean up the created file
    std::remove(filename.c_str());

    return 0;
}