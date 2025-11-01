
#include <iostream>
#include <cstring>
#include <cstdio>

int scanString(char* buffer, int maxSize) {
    if (buffer == nullptr || maxSize <= 0) {
        return -1;
    }
    
    // Clear buffer
    memset(buffer, 0, maxSize);
    
    // Use fgets for safe input - reads at most maxSize-1 characters
    if (fgets(buffer, maxSize, stdin) == nullptr) {
        return -1;
    }
    
    // Remove newline if present
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\\n') {
        buffer[len - 1] = '\\0';
        len--;
    }
    
    return static_cast<int>(len);
}

int main() {
    const int BUFFER_SIZE = 1024;
    
    std::cout << "Test Case 1: Short string" << std::endl;
    char buffer1[BUFFER_SIZE];
    std::cout << "Enter input: ";
    int count1 = scanString(buffer1, BUFFER_SIZE);
    std::cout << "Characters read: " << count1 << std::endl;
    std::cout << "Buffer content: " << buffer1 << std::endl;
    
    std::cout << "\\nTest Case 2: Empty string" << std::endl;
    char buffer2[BUFFER_SIZE];
    std::cout << "Enter input: ";
    int count2 = scanString(buffer2, BUFFER_SIZE);
    std::cout << "Characters read: " << count2 << std::endl;
    
    std::cout << "\\nTest Case 3: String with spaces" << std::endl;
    char buffer3[BUFFER_SIZE];
    std::cout << "Enter input: ";
    int count3 = scanString(buffer3, BUFFER_SIZE);
    std::cout << "Characters read: " << count3 << std::endl;
    std::cout << "Buffer content: " << buffer3 << std::endl;
    
    std::cout << "\\nTest Case 4: Maximum length string" << std::endl;
    char buffer4[BUFFER_SIZE];
    std::cout << "Enter input: ";
    int count4 = scanString(buffer4, BUFFER_SIZE);
    std::cout << "Characters read: " << count4 << std::endl;
    
    std::cout << "\\nTest Case 5: Special characters" << std::endl;
    char buffer5[BUFFER_SIZE];
    std::cout << "Enter input: ";
    int count5 = scanString(buffer5, BUFFER_SIZE);
    std::cout << "Characters read: " << count5 << std::endl;
    std::cout << "Buffer content: " << buffer5 << std::endl;
    
    return 0;
}
