
#include <iostream>
#include <cstring>
#include <memory>
#include <string>

struct BufferResult {
    std::string buf1;
    std::string buf2;
};

BufferResult allocateAndReadBuffers(const char* input1, const char* input2) {
    // Security: Define buffer size constants
    const int BUFFER_SIZE = 1024;
    const int READ_SIZE = 100;
    
    // Allocate first buffer with smart pointer for automatic cleanup
    std::unique_ptr<char[]> buf(new char[BUFFER_SIZE]);
    std::memset(buf.get(), 0, BUFFER_SIZE);
    
    // Allocate second buffer
    std::unique_ptr<char[]> buf1(new char[BUFFER_SIZE]);
    std::memset(buf1.get(), 0, BUFFER_SIZE);
    
    BufferResult result;
    
    // Security: Validate and limit input size to prevent overflow
    if (input1 != nullptr) {
        size_t len = std::strlen(input1);
        size_t copy_len = (len > READ_SIZE) ? READ_SIZE : len;
        std::strncpy(buf.get(), input1, copy_len);
        buf[copy_len] = '\\0'; // Ensure null termination
        result.buf1 = std::string(buf.get());
    }
    
    if (input2 != nullptr) {
        size_t len = std::strlen(input2);
        size_t copy_len = (len > READ_SIZE) ? READ_SIZE : len;
        std::strncpy(buf1.get(), input2, copy_len);
        buf1[copy_len] = '\\0'; // Ensure null termination
        result.buf2 = std::string(buf1.get());
    }
    
    return result;
}

int main() {
    // Test case 1: Normal input
    BufferResult result1 = allocateAndReadBuffers("Hello World", "Test Input");
    std::cout << "Test 1 - Buf1: " << result1.buf1 << ", Buf2: " << result1.buf2 << std::endl;
    
    // Test case 2: Empty strings
    BufferResult result2 = allocateAndReadBuffers("", "");
    std::cout << "Test 2 - Buf1: '" << result2.buf1 << "', Buf2: '" << result2.buf2 << "'" << std::endl;
    
    // Test case 3: Null inputs
    BufferResult result3 = allocateAndReadBuffers(nullptr, nullptr);
    std::cout << "Test 3 - Buf1: '" << result3.buf1 << "', Buf2: '" << result3.buf2 << "'" << std::endl;
    
    // Test case 4: Input exceeding 100 characters (should be truncated)
    std::string longString(150, 'A');
    BufferResult result4 = allocateAndReadBuffers(longString.c_str(), longString.c_str());
    std::cout << "Test 4 - Buf1 length: " << result4.buf1.length() 
              << ", Buf2 length: " << result4.buf2.length() << std::endl;
    
    // Test case 5: Exactly 100 characters
    std::string exactString(100, 'B');
    BufferResult result5 = allocateAndReadBuffers(exactString.c_str(), exactString.c_str());
    std::cout << "Test 5 - Buf1 length: " << result5.buf1.length() 
              << ", Buf2 length: " << result5.buf2.length() << std::endl;
    
    return 0;
}
