
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_PASSWORD_LENGTH 1024
#define MAX_FILENAME_LENGTH 255
#define MAX_FILE_SIZE 2048

// Constant-time comparison to prevent timing attacks
bool constantTimeCompare(const char* a, size_t aLen, const char* b, size_t bLen) {
    if (a == NULL || b == NULL) {
        return false;
    }
    
    // Always compare to prevent length-based timing leaks
    if (aLen != bLen) {
        return false;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < aLen; i++) {
        result |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }
    
    return result == 0;
}

// Validates password input constraints
bool validatePasswordInput(const char* password, size_t length) {
    if (password == NULL || length == 0 || length > MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    // Check for null bytes in password
    for (size_t i = 0; i < length; i++) {
        if (password[i] == '\\0') {
            return false;
        }
    }
    
    return true;
}

// Validates filename to prevent path traversal
bool validateFilename(const char* filename) {
    if (filename == NULL) {
        return false;
    }
    
    size_t len = strnlen(filename, MAX_FILENAME_LENGTH + 1);
    if (len == 0 || len > MAX_FILENAME_LENGTH) {
        return false;
    }
    
    // Check for path traversal attempts
    if (strstr(filename, "..") != NULL || 
        strchr(filename, '/') != NULL ||
        strchr(filename, '\\\\') != NULL) {\n        return false;\n    }\n    \n    return true;\n}\n\n// Securely reads password from file with validation\nbool readPasswordFromFile(const char* filename, char* storedPassword, size_t* passwordLength) {\n    if (filename == NULL || storedPassword == NULL || passwordLength == NULL) {\n        return false;\n    }\n    \n    if (!validateFilename(filename)) {\n        return false;\n    }\n    \n    FILE* file = fopen(filename, "rb");\n    if (file == NULL) {\n        return false;\n    }\n    \n    // Read with bounds checking\n    char buffer[MAX_FILE_SIZE + 1];\n    memset(buffer, 0, sizeof(buffer));\n    \n    size_t bytesRead = fread(buffer, 1, MAX_FILE_SIZE, file);\n    \n    // Check for read errors\n    if (ferror(file)) {\n        fclose(file);\n        return false;\n    }\n    \n    fclose(file);\n    \n    if (bytesRead == 0 || bytesRead > MAX_FILE_SIZE) {\n        return false;\n    }\n    \n    // Ensure null termination\n    buffer[bytesRead] = '\\0';\n    \n    // Remove trailing newline characters\n    while (bytesRead > 0 && (buffer[bytesRead - 1] == '\
' || buffer[bytesRead - 1] == '\\r')) {\n        bytesRead--;\n        buffer[bytesRead] = '\\0';\n    }\n    \n    // Validate stored password\n    if (!validatePasswordInput(buffer, bytesRead)) {\n        memset(buffer, 0, sizeof(buffer));\n        return false;\n    }\n    \n    // Copy to output with bounds checking\n    if (bytesRead > MAX_PASSWORD_LENGTH) {\n        memset(buffer, 0, sizeof(buffer));\n        return false;\n    }\n    \n    memcpy(storedPassword, buffer, bytesRead);\n    storedPassword[bytesRead] = '\\0';
    *passwordLength = bytesRead;
    
    // Clear sensitive data
    memset(buffer, 0, sizeof(buffer));
    
    return true;
}

// Main password verification function
bool verifyPassword(const char* filename, const char* userPassword) {
    if (filename == NULL || userPassword == NULL) {
        return false;
    }
    
    size_t userPasswordLen = strnlen(userPassword, MAX_PASSWORD_LENGTH + 1);
    
    // Validate user input
    if (!validatePasswordInput(userPassword, userPasswordLen)) {
        return false;
    }
    
    char storedPassword[MAX_PASSWORD_LENGTH + 1];
    memset(storedPassword, 0, sizeof(storedPassword));
    size_t storedPasswordLen = 0;
    
    if (!readPasswordFromFile(filename, storedPassword, &storedPasswordLen)) {
        memset(storedPassword, 0, sizeof(storedPassword));
        return false;
    }
    
    // Use constant-time comparison
    bool result = constantTimeCompare(storedPassword, storedPasswordLen, 
                                     userPassword, userPasswordLen);
    
    // Securely clear stored password from memory
    memset(storedPassword, 0, sizeof(storedPassword));
    
    return result;
}

int main(void) {
    // Test case 1: Valid password match
    {
        FILE* testFile = fopen("test1.txt", "wb");
        if (testFile != NULL) {
            fprintf(testFile, "SecurePass123");
            fclose(testFile);
        }
        
        bool result = verifyPassword("test1.txt", "SecurePass123");
        printf("Test 1 (valid match): %s\\n", result ? "PASS" : "FAIL");
    }
    
    // Test case 2: Invalid password (mismatch)
    {
        FILE* testFile = fopen("test2.txt", "wb");
        if (testFile != NULL) {
            fprintf(testFile, "CorrectPassword");
            fclose(testFile);
        }
        
        bool result = verifyPassword("test2.txt", "WrongPassword");
        printf("Test 2 (mismatch): %s\\n", !result ? "PASS" : "FAIL");
    }
    
    // Test case 3: Empty password attempt
    {
        FILE* testFile = fopen("test3.txt", "wb");
        if (testFile != NULL) {
            fprintf(testFile, "ValidPass");
            fclose(testFile);
        }
        
        bool result = verifyPassword("test3.txt", "");
        printf("Test 3 (empty input): %s\\n", !result ? "PASS" : "FAIL");
    }
    
    // Test case 4: File with newline
    {
        FILE* testFile = fopen("test4.txt", "wb");
        if (testFile != NULL) {
            fprintf(testFile, "PasswordWithNewline\\n");
            fclose(testFile);
        }
        
        bool result = verifyPassword("test4.txt", "PasswordWithNewline");
        printf("Test 4 (newline handling): %s\\n", result ? "PASS" : "FAIL");
    }
    
    // Test case 5: Path traversal attempt (security test)
    {
        bool result = verifyPassword("../etc/passwd", "anypassword");
        printf("Test 5 (path traversal blocked): %s\\n", !result ? "PASS" : "FAIL");
    }
    
    return 0;
}
