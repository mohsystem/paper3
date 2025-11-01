
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <stdexcept>

// Secure zeroization function that compiler cannot optimize away
static void secure_zero(void* ptr, size_t len) {
    if (ptr == nullptr || len == 0) return;
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) {
        *p++ = 0;
    }
}

// Allocate two chunks of memory, copy first to second, and return second
char* allocateAndCopy(const char* sourceData) {
    if (sourceData == nullptr) {
        char* empty = static_cast<char*>(malloc(1));
        if (empty == nullptr) {
            throw std::runtime_error("Memory allocation failed");
        }
        empty[0] = '\\0';
        return empty;
    }
    
    // Step 1: Calculate size with bounds checking
    size_t dataLen = strlen(sourceData);
    
    // Prevent integer overflow and excessive allocation (1MB limit)
    if (dataLen > 1048576 || dataLen >= SIZE_MAX - 1) {
        throw std::runtime_error("Data size exceeds maximum allowed");
    }
    
    size_t allocSize = dataLen + 1; // +1 for null terminator
    
    // Step 2: Allocate first chunk
    char* firstChunk = static_cast<char*>(malloc(allocSize));
    if (firstChunk == nullptr) {
        throw std::runtime_error("Memory allocation failed for first chunk");
    }
    
    // Step 3: Copy source data to first chunk with bounds checking
    strncpy(firstChunk, sourceData, dataLen);
    firstChunk[dataLen] = '\\0'; // Ensure null termination
    
    // Step 4: Allocate second chunk
    char* secondChunk = static_cast<char*>(malloc(allocSize));
    if (secondChunk == nullptr) {
        secure_zero(firstChunk, allocSize);
        free(firstChunk);
        throw std::runtime_error("Memory allocation failed for second chunk");
    }
    
    // Step 5: Copy from first chunk to second chunk
    memcpy(secondChunk, firstChunk, allocSize);
    
    // Step 6: Clear and free first chunk (it contained the data)
    secure_zero(firstChunk, allocSize);
    free(firstChunk);
    
    return secondChunk;
}

int main() {
    char* result = nullptr;
    
    // Test case 1: Normal string
    result = allocateAndCopy("Hello World");
    std::cout << "Test 1: " << result << std::endl;
    secure_zero(result, strlen(result) + 1);
    free(result);
    
    // Test case 2: Empty string
    result = allocateAndCopy("");
    std::cout << "Test 2: " << result << std::endl;
    free(result);
    
    // Test case 3: String with special characters
    result = allocateAndCopy("Test@123!#$");
    std::cout << "Test 3: " << result << std::endl;
    secure_zero(result, strlen(result) + 1);
    free(result);
    
    // Test case 4: nullptr input
    result = allocateAndCopy(nullptr);
    std::cout << "Test 4: " << result << std::endl;
    free(result);
    
    // Test case 5: Longer string
    result = allocateAndCopy("The quick brown fox jumps over the lazy dog");
    std::cout << "Test 5: " << result << std::endl;
    secure_zero(result, strlen(result) + 1);
    free(result);
    
    return 0;
}
