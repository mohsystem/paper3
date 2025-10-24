
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Constant-time string comparison to prevent timing attacks
bool constantTimeEquals(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return false;
    }
    
    size_t lengthA = strlen(a);
    size_t lengthB = strlen(b);
    
    // Use the longer length to prevent timing attacks
    size_t maxLength = (lengthA > lengthB) ? lengthA : lengthB;
    int result = lengthA ^ lengthB; // Different lengths contribute to mismatch
    
    for (size_t i = 0; i < maxLength; i++) {
        char charA = (i < lengthA) ? a[i] : 0;
        char charB = (i < lengthB) ? b[i] : 0;
        result |= charA ^ charB;
    }
    
    return result == 0;
}

// Trim whitespace from string
void trim(char* str) {
    if (str == NULL) return;
    
    // Trim trailing whitespace
    size_t len = strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\\t' || 
           str[len - 1] == '\\n' || str[len - 1] == '\\r')) {
        str[--len] = '\\0';
    }
    
    // Trim leading whitespace
    char* start = str;
    while (*start && (*start == ' ' || *start == '\\t' || 
           *start == '\\n' || *start == '\\r')) {
        start++;
    }
    
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

bool verifyPassword(const char* filePath, const char* enteredPassword) {
    FILE* file = fopen(filePath, "r");
    
    if (file == NULL) {
        fprintf(stderr, "Error: Could not open password file\\n");
        return false;
    }
    
    char storedPassword[256];
    memset(storedPassword, 0, sizeof(storedPassword));
    
    if (fgets(storedPassword, sizeof(storedPassword), file) == NULL) {
        fprintf(stderr, "Error: Could not read password from file\\n");
        fclose(file);
        return false;
    }
    
    fclose(file);
    
    // Trim whitespace
    trim(storedPassword);
    
    // Constant-time comparison
    bool matches = constantTimeEquals(storedPassword, enteredPassword);
    
    // Clear sensitive data from memory
    memset(storedPassword, 0, sizeof(storedPassword));
    
    return matches;
}

int main() {
    printf("=== Password Verification Test Cases ===\\n\\n");
    
    // Create test password file
    const char* testFile = "test_password.txt";
    FILE* outFile = fopen(testFile, "w");
    if (outFile != NULL) {
        fprintf(outFile, "SecurePass123!");
        fclose(outFile);
    } else {
        fprintf(stderr, "Failed to create test file\\n");
        return 1;
    }
    
    // Test Case 1: Correct password
    printf("Test 1 - Correct password:\\n");
    bool result1 = verifyPassword(testFile, "SecurePass123!");
    printf("Result: %s\\n\\n", result1 ? "MATCH" : "NO MATCH");
    
    // Test Case 2: Incorrect password
    printf("Test 2 - Incorrect password:\\n");
    bool result2 = verifyPassword(testFile, "WrongPassword");
    printf("Result: %s\\n\\n", result2 ? "MATCH" : "NO MATCH");
    
    // Test Case 3: Password with different case
    printf("Test 3 - Different case:\\n");
    bool result3 = verifyPassword(testFile, "securepass123!");
    printf("Result: %s\\n\\n", result3 ? "MATCH" : "NO MATCH");
    
    // Test Case 4: Empty password
    printf("Test 4 - Empty password:\\n");
    bool result4 = verifyPassword(testFile, "");
    printf("Result: %s\\n\\n", result4 ? "MATCH" : "NO MATCH");
    
    // Test Case 5: Password with extra characters
    printf("Test 5 - Password with extra characters:\\n");
    bool result5 = verifyPassword(testFile, "SecurePass123!Extra");
    printf("Result: %s\\n\\n", result5 ? "MATCH" : "NO MATCH");
    
    // Cleanup test file
    remove(testFile);
    
    return 0;
}
