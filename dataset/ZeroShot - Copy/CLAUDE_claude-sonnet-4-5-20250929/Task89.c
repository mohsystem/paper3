
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

bool comparePasswords(const char* storedPassword, const char* enteredPassword) {
    if (storedPassword == NULL || enteredPassword == NULL) {
        return false;
    }
    
    size_t storedLen = strlen(storedPassword);
    size_t enteredLen = strlen(enteredPassword);
    
    if (storedLen != enteredLen) {
        return false;
    }
    
    for (size_t i = 0; i < storedLen; i++) {
        if (storedPassword[i] != enteredPassword[i]) {
            return false;
        }
    }
    
    return true;
}

bool readPasswordFromFile(const char* filename, char* buffer, size_t bufferSize) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error reading password file\\n");
        return false;
    }
    
    if (fgets(buffer, bufferSize, file) != NULL) {
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\\n') {
            buffer[len - 1] = '\\0';
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
    // Create a test password file
    const char* testFile = "password.txt";
    FILE* outFile = fopen(testFile, "w");
    if (outFile != NULL) {
        fprintf(outFile, "SecurePass123\\n");
        fclose(outFile);
    }
    
    // Test cases
    printf("Test Case 1 - Correct password:\\n");
    bool result1 = verifyPassword(testFile, "SecurePass123");
    printf("Result: %s\\n", result1 ? "Access Granted" : "Access Denied");
    
    printf("\\nTest Case 2 - Incorrect password:\\n");
    bool result2 = verifyPassword(testFile, "WrongPass456");
    printf("Result: %s\\n", result2 ? "Access Granted" : "Access Denied");
    
    printf("\\nTest Case 3 - Empty password:\\n");
    bool result3 = verifyPassword(testFile, "");
    printf("Result: %s\\n", result3 ? "Access Granted" : "Access Denied");
    
    printf("\\nTest Case 4 - Case sensitive check:\\n");
    bool result4 = verifyPassword(testFile, "securepass123");
    printf("Result: %s\\n", result4 ? "Access Granted" : "Access Denied");
    
    printf("\\nTest Case 5 - Partial match:\\n");
    bool result5 = verifyPassword(testFile, "SecurePass");
    printf("Result: %s\\n", result5 ? "Access Granted" : "Access Denied");
    
    // Clean up test file
    remove(testFile);
    
    return 0;
}
