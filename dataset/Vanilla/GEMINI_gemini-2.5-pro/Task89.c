#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * @brief Creates a dummy file with the given password.
 * @param filename The name of the file to create.
 * @param password The password to store in the file.
 */
void createPasswordFile(const char* filename, const char* password) {
    FILE* fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("Error creating password file");
        return;
    }
    fprintf(fp, "%s", password);
    fclose(fp);
}

/**
 * @brief Compares the password in a file with the user-provided password
 * by checking each character sequentially.
 * @param filename The name of the file containing the stored password.
 * @param userInput The password entered by the user.
 * @return true if the passwords match, false otherwise.
 */
bool comparePassword(const char* filename, const char* userInput) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Error reading password file");
        return false;
    }

    char storedPassword[256] = {0}; // Buffer to hold the password from file
    if (fgets(storedPassword, sizeof(storedPassword), fp) == NULL) {
        // Handle file read error or empty file
        fclose(fp);
        // If user input is also empty, it's a match
        return strlen(userInput) == 0;
    }
    fclose(fp);
    
    // Remove newline character if present (fgets includes it)
    storedPassword[strcspn(storedPassword, "\r\n")] = 0;

    // First, check if the lengths are the same.
    size_t storedLen = strlen(storedPassword);
    size_t inputLen = strlen(userInput);
    if (storedLen != inputLen) {
        return false;
    }

    // Compare each character sequentially
    for (size_t i = 0; i < storedLen; ++i) {
        if (storedPassword[i] != userInput[i]) {
            return false; // Mismatch found
        }
    }

    // If the loop completes, all characters matched
    return true;
}

int main() {
    const char* filename = "password.txt";
    const char* correctPassword = "P@ssw0rd123";

    // Create the password file for testing
    createPasswordFile(filename, correctPassword);

    // --- Test Cases ---
    const char* testPasswords[] = {
        "P@ssw0rd123",    // 1. Correct password
        "p@ssw0rd123",    // 2. Incorrect case
        "P@ssw0rd",       // 3. Incorrect length (shorter)
        "P@ssw0rd124",    // 4. Same length, different character
        ""                // 5. Empty password
    };
    int num_tests = sizeof(testPasswords) / sizeof(testPasswords[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        bool isMatch = comparePassword(filename, testPasswords[i]);
        printf("Testing with \"%s\": %s\n", testPasswords[i], isMatch ? "Match" : "No Match");
    }

    // Clean up the created file
    remove(filename);

    return 0;
}