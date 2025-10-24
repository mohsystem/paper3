#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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
bool comparePassword(const char* filename, const char* userInput) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening password file");
        return false;
    }

    char storedPassword[256];
    if (fgets(storedPassword, sizeof(storedPassword), file) == NULL) {
        // Handle empty file or read error
        storedPassword[0] = '\0';
    }
    fclose(file);

    // Remove newline character that fgets might have read
    storedPassword[strcspn(storedPassword, "\n")] = 0;

    size_t storedLen = strlen(storedPassword);
    size_t userLen = strlen(userInput);

    // Length check is performed first. This can leak password length information.
    if (storedLen != userLen) {
        return false;
    }

    // Constant-time comparison to prevent timing attacks.
    int diff = 0;
    for (size_t i = 0; i < storedLen; ++i) {
        // XOR the characters and OR the result into diff.
        diff |= storedPassword[i] ^ userInput[i];
    }

    return diff == 0;
}

int main() {
    const char* testFilename = "password.txt";
    const char* correctPassword = "S3cur3P@ssw0rd!";

    // Setup: Create a dummy password file for testing
    FILE* outFile = fopen(testFilename, "w");
    if (outFile == NULL) {
        perror("Failed to create test password file");
        return 1;
    }
    fprintf(outFile, "%s", correctPassword);
    fclose(outFile);

    printf("--- C Test Cases ---\n");
    // Test Case 1: Correct password
    bool result1 = comparePassword(testFilename, "S3cur3P@ssw0rd!");
    printf("Test 1 (Correct): %s\n", result1 ? "PASS" : "FAIL");

    // Test Case 2: Incorrect password (different first character)
    bool result2 = comparePassword(testFilename, "X3cur3P@ssw0rd!");
    printf("Test 2 (Incorrect, first char): %s\n", !result2 ? "PASS" : "FAIL");

    // Test Case 3: Incorrect password (different last character)
    bool result3 = comparePassword(testFilename, "S3cur3P@ssw0rd?");
    printf("Test 3 (Incorrect, last char): %s\n", !result3 ? "PASS" : "FAIL");

    // Test Case 4: Incorrect password (shorter)
    bool result4 = comparePassword(testFilename, "S3cur3");
    printf("Test 4 (Incorrect, shorter): %s\n", !result4 ? "PASS" : "FAIL");

    // Test Case 5: Incorrect password (longer)
    bool result5 = comparePassword(testFilename, "S3cur3P@ssw0rd!Extra");
    printf("Test 5 (Incorrect, longer): %s\n", !result5 ? "PASS" : "FAIL");

    // Cleanup: Delete the test file
    remove(testFilename);

    return 0;
}