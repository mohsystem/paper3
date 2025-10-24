#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Note: Storing plaintext passwords is a major security risk.
// In a real-world application, you should store a strong cryptographic hash
// of the password (e.g., using Argon2, scrypt, or bcrypt) and compare the hashes.
// The prompt describes an insecure, sequential comparison which is vulnerable
// to timing attacks. This code implements a secure, constant-time comparison.

/**
 * Securely compares two passwords in a way that prevents timing attacks.
 * This function performs a constant-time comparison for strings of the same length.
 *
 * @param userInputPassword The password entered by the user.
 * @param storedPassword    The password that would be read from a file or database.
 * @return true if the passwords match, false otherwise.
 */
bool securePasswordCompare(const char* userInputPassword, const char* storedPassword) {
    if (userInputPassword == NULL || storedPassword == NULL) {
        return false;
    }

    size_t userLen = strlen(userInputPassword);
    size_t storedLen = strlen(storedPassword);

    // Check for length mismatch. While this leaks length information, it is a
    // common and generally accepted practice, as the length of a hash is
    // typically public knowledge anyway.
    if (userLen != storedLen) {
        return false;
    }

    // Use a variable to accumulate differences without short-circuiting.
    // The comparison will always take the same amount of time for strings of the same length.
    int diff = 0;
    for (size_t i = 0; i < storedLen; ++i) {
        // XOR the characters and OR the result with the accumulator.
        // If characters are the same, (c1 ^ c2) is 0.
        // If they are different, it's non-zero.
        // `diff` will be 0 only if all characters matched.
        diff |= userInputPassword[i] ^ storedPassword[i];
    }

    // Return true only if the accumulator is still 0.
    return diff == 0;
}


void run_test(const char* test_name, bool result, bool expected) {
    printf("%s: %s\n", test_name, (result == expected) ? "PASS" : "FAIL");
}

int main() {
    // This would be the password stored securely (as a hash in a real app).
    const char* storedPassword = "S3cureP@ssw0rd!23";

    printf("--- C: Test Cases for Secure Password Comparison ---\n");

    // Test Case 1: Correct password
    const char* input1 = "S3cureP@ssw0rd!23";
    run_test("Test 1 (Correct Match)", securePasswordCompare(input1, storedPassword), true);

    // Test Case 2: Incorrect password (different case)
    const char* input2 = "s3curep@ssw0rd!23";
    run_test("Test 2 (Case Mismatch)", securePasswordCompare(input2, storedPassword), false);

    // Test Case 3: Incorrect password (different length)
    const char* input3 = "S3cureP@ssw0rd!";
    run_test("Test 3 (Length Mismatch)", securePasswordCompare(input3, storedPassword), false);

    // Test Case 4: Incorrect password (same length, different char)
    const char* input4 = "S3cureP@ssw0rd!24";
    run_test("Test 4 (Character Mismatch)", securePasswordCompare(input4, storedPassword), false);

    // Test Case 5: Empty string vs stored password
    const char* input5 = "";
    run_test("Test 5 (Empty Input)", securePasswordCompare(input5, storedPassword), false);

    return 0;
}