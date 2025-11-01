
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <memory>

// Secure memory allocation with RAII for automatic cleanup
struct SecureBuffer {
    char* data;
    size_t size;
    
    SecureBuffer(size_t sz) : data(nullptr), size(sz) {
        // Allocate memory and check for allocation failure
        data = static_cast<char*>(std::calloc(sz, 1));
        if (!data) {
            throw std::bad_alloc();
        }
    }
    
    ~SecureBuffer() {
        if (data) {
            // Clear sensitive data before freeing - use volatile to prevent optimization
            volatile char* vdata = data;
            for (size_t i = 0; i < size; ++i) {
                vdata[i] = 0;
            }
            std::free(data);
            data = nullptr;
        }
    }
    
    // Prevent copying to avoid double-free
    SecureBuffer(const SecureBuffer&) = delete;
    SecureBuffer& operator=(const SecureBuffer&) = delete;
};

void memoryAllocationMethod(char* out1, size_t out1_size, char* out2, size_t out2_size) {
    // Initialize output buffers to ensure null termination
    if (out1 && out1_size > 0) {
        std::memset(out1, 0, out1_size);
    }
    if (out2 && out2_size > 0) {
        std::memset(out2, 0, out2_size);
    }
    
    try {
        // Allocate first buffer - using RAII for automatic cleanup
        SecureBuffer buf(1024);
        
        // Read exactly 100 characters from stdin with bounds checking
        // Using fgets instead of gets/scanf for safety
        if (std::fgets(buf.data, 101, stdin) == nullptr) {
            // Handle read error - fail closed
            std::cerr << "Input error occurred" << std::endl;
            return;
        }
        
        // Ensure null termination
        buf.data[100] = '\\0';
        
        // Safely copy to output buffer with size check
        if (out1 && out1_size > 0) {
            size_t copy_len = std::min(static_cast<size_t>(100), out1_size - 1);
            std::memcpy(out1, buf.data, copy_len);
            out1[copy_len] = '\\0';
        }
        
        // Allocate second buffer - using RAII for automatic cleanup
        SecureBuffer buf1(1024);
        
        // Read exactly 100 characters from stdin with bounds checking
        if (std::fgets(buf1.data, 101, stdin) == nullptr) {
            // Handle read error - fail closed
            std::cerr << "Input error occurred" << std::endl;
            return;
        }
        
        // Ensure null termination
        buf1.data[100] = '\\0';
        
        // Safely copy to output buffer with size check
        if (out2 && out2_size > 0) {
            size_t copy_len = std::min(static_cast<size_t>(100), out2_size - 1);
            std::memcpy(out2, buf1.data, copy_len);
            out2[copy_len] = '\\0';
        }
        
        // Buffers automatically cleaned up by RAII destructors
        
    } catch (const std::bad_alloc&) {
        std::cerr << "Memory allocation failed" << std::endl;
    }
}

int main() {
    char output1[1024] = {0};
    char output2[1024] = {0};
    
    // Test case 1: Normal input
    std::cout << "Test 1: Normal input" << std::endl;
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    std::cout << "Buffer 1 length: " << std::strlen(output1) << std::endl;
    std::cout << "Buffer 2 length: " << std::strlen(output2) << std::endl;
    
    // Test case 2: Empty input
    std::cout << "\\nTest 2: Empty input" << std::endl;
    std::memset(output1, 0, sizeof(output1));
    std::memset(output2, 0, sizeof(output2));
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    std::cout << "Buffer 1 length: " << std::strlen(output1) << std::endl;
    std::cout << "Buffer 2 length: " << std::strlen(output2) << std::endl;
    
    // Test case 3: Short input
    std::cout << "\\nTest 3: Short input" << std::endl;
    std::memset(output1, 0, sizeof(output1));
    std::memset(output2, 0, sizeof(output2));
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    std::cout << "Buffer 1 length: " << std::strlen(output1) << std::endl;
    std::cout << "Buffer 2 length: " << std::strlen(output2) << std::endl;
    
    // Test case 4: Exactly 100 characters
    std::cout << "\\nTest 4: Exactly 100 characters" << std::endl;
    std::memset(output1, 0, sizeof(output1));
    std::memset(output2, 0, sizeof(output2));
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    std::cout << "Buffer 1 length: " << std::strlen(output1) << std::endl;
    std::cout << "Buffer 2 length: " << std::strlen(output2) << std::endl;
    
    // Test case 5: More than 100 characters
    std::cout << "\\nTest 5: More than 100 characters" << std::endl;
    std::memset(output1, 0, sizeof(output1));
    std::memset(output2, 0, sizeof(output2));
    memoryAllocationMethod(output1, sizeof(output1), output2, sizeof(output2));
    std::cout << "Buffer 1 length: " << std::strlen(output1) << std::endl;
    std::cout << "Buffer 2 length: " << std::strlen(output2) << std::endl;
    
    return 0;
}
