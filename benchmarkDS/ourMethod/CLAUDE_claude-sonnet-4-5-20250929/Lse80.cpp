
#include <cstdio>
#include <cstring>
#include <iostream>

// Reads a line from standard input with maximum length limit
// Returns number of characters read (excluding newline)
// Maximum 1023 characters to fit in 1024-byte buffer (with null terminator)
int readLimitedLine(char* buffer, int maxLen) {
    // Validate input parameters - bounds checking to prevent buffer overflow
    if (buffer == nullptr || maxLen <= 0) {
        return -1; // Fail closed on invalid input
    }
    
    // Initialize buffer to zeros for safety
    memset(buffer, 0, maxLen + 1);
    
    // Use fgets for safe input with explicit length limit
    // fgets reads at most maxLen characters and adds null terminator
    if (fgets(buffer, maxLen + 1, stdin) == nullptr) {
        return 0; // No input or error
    }
    
    // Calculate actual characters read
    size_t len = strlen(buffer);
    
    // Remove trailing newline if present - newline not stored per requirement
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    if (len > 0 && buffer[len - 1] == '\\r') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    // Enforce maximum length - additional safety check
    if (len > static_cast<size_t>(maxLen)) {
        len = maxLen;
        buffer[maxLen] = '\\0';
    }
    
    return static_cast<int>(len);
}

int main() {
    const int BUFFER_SIZE = 1024;
    const int MAX_INPUT = 1023;
    
    std::cout << "Test Case 1: Normal input (type 'Hello' and press Enter)\\n";
    char buffer1[BUFFER_SIZE] = {0};
    int result1 = readLimitedLine(buffer1, MAX_INPUT);
    std::cout << "Characters read: " << result1 << ", Content: '" << buffer1 << "'\\n\\n";
    
    std::cout << "Test Case 2: Empty input (press Enter only)\\n";
    char buffer2[BUFFER_SIZE] = {0};
    int result2 = readLimitedLine(buffer2, MAX_INPUT);
    std::cout << "Characters read: " << result2 << ", Content: '" << buffer2 << "'\\n\\n";
    
    std::cout << "Test Case 3: Special characters (type 'Test@123!#')\\n";
    char buffer3[BUFFER_SIZE] = {0};
    int result3 = readLimitedLine(buffer3, MAX_INPUT);
    std::cout << "Characters read: " << result3 << ", Content: '" << buffer3 << "'\\n\\n";
    
    std::cout << "Test Case 4: Long input (type many characters)\\n";
    char buffer4[BUFFER_SIZE] = {0};
    int result4 = readLimitedLine(buffer4, MAX_INPUT);
    std::cout << "Characters read: " << result4 << " (max " << MAX_INPUT << ")\\n\\n";
    
    std::cout << "Test Case 5: Whitespace (type spaces)\\n";
    char buffer5[BUFFER_SIZE] = {0};
    int result5 = readLimitedLine(buffer5, MAX_INPUT);
    std::cout << "Characters read: " << result5 << ", Content: '" << buffer5 << "'\\n";
    
    return 0;
}
