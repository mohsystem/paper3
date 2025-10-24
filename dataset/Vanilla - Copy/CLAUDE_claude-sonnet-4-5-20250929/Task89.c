
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool comparePasswords(const char* storedPassword, const char* enteredPassword) {
    if (storedPassword == NULL || enteredPassword == NULL) {
        return false;
    }
    
    size_t len1 = strlen(storedPassword);
    size_t len2 = strlen(enteredPassword);
    
    if (len1 != len2) {
        return false;
    }
    
    for (size_t i = 0; i < len1; i++) {
        if (storedPassword[i] != enteredPassword[i]) {
            return false;
        }
    }
    
    return true;
}

bool readPasswordFromFile(const char* filename, char* password, size_t maxLen) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        return false;
    }
    
    if (fgets(password, maxLen, file) != NULL) {
        size_t len = strlen(password);
        if (len > 0 && password[len - 1] == '\\n') {
            password[len - 1] = '\\0';
        }
        fclose(file);
        return true;
    }
    
    fclose(file);
    return false;
}

bool verifyPassword(const char* filename, const char* enteredPassword) {
    char storedPassword[256];
    
    if (!readPasswordFromFile(filename, storedPassword, sizeof(storedPassword))) {
        return false;
    }
    
    return comparePasswords(storedPassword, enteredPassword);
}

int main() {
    // Create test file with password
    FILE* outFile = fopen("password.txt", "w");
    if (outFile != NULL) {
        fprintf(outFile, "SecurePass123");
        fclose(outFile);
    }
    
    // Test Case 1: Correct password
    printf("Test 1 - Correct password:\\n");
    bool result1 = verifyPassword("password.txt", "SecurePass123");
    printf("Result: %s (Expected: true)\\n\\n", result1 ? "true" : "false");
    
    // Test Case 2: Incorrect password
    printf("Test 2 - Incorrect password:\\n");
    bool result2 = verifyPassword("password.txt", "WrongPass123");
    printf("Result: %s (Expected: false)\\n\\n", result2 ? "true" : "false");
    
    // Test Case 3: Password with different length
    printf("Test 3 - Different length:\\n");
    bool result3 = verifyPassword("password.txt", "Short");
    printf("Result: %s (Expected: false)\\n\\n", result3 ? "true" : "false");
    
    // Test Case 4: Empty password
    printf("Test 4 - Empty password:\\n");
    bool result4 = verifyPassword("password.txt", "");
    printf("Result: %s (Expected: false)\\n\\n", result4 ? "true" : "false");
    
    // Test Case 5: Case sensitive check
    printf("Test 5 - Case sensitive:\\n");
    bool result5 = verifyPassword("password.txt", "securepass123");
    printf("Result: %s (Expected: false)\\n\\n", result5 ? "true" : "false");
    
    return 0;
}
