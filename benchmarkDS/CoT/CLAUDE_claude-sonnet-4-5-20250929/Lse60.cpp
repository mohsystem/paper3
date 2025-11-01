
#include <iostream>
#include <vector>
#include <cstring>
#include <limits>

const size_t MAX_BUFFER_SIZE = 1024 * 1024; // 1MB limit

std::string allocateAndPrintBuffer(long long size) {
    // Input validation
    if (size <= 0) {
        return "Error: Buffer size must be positive";
    }
    
    if (size > static_cast<long long>(MAX_BUFFER_SIZE)) {
        return "Error: Buffer size exceeds maximum allowed (" + 
               std::to_string(MAX_BUFFER_SIZE) + " bytes)";
    }
    
    try {
        // Allocate buffer and initialize with zeros for security
        std::vector<unsigned char> buffer(static_cast<size_t>(size), 0);
        
        // Print buffer contents (safely, limit output)
        std::string result = "Buffer contents (" + std::to_string(size) + " bytes): [";
        size_t display_size = (size < 100) ? size : 100;
        
        for (size_t i = 0; i < display_size; i++) {
            result += std::to_string(buffer[i]);
            if (i < display_size - 1) result += ", ";
        }
        if (size > 100) result += "...";
        result += "]";
        
        return result;
    } catch (const std::bad_alloc& e) {
        return "Error: Unable to allocate buffer - out of memory";
    }
}

int main(int argc, char* argv[]) {
    // Test cases
    std::cout << "Test Case 1 (valid size 10):\\n";
    std::cout << allocateAndPrintBuffer(10) << "\\n\\n";
    
    std::cout << "Test Case 2 (valid size 50):\\n";
    std::cout << allocateAndPrintBuffer(50) << "\\n\\n";
    
    std::cout << "Test Case 3 (negative size):\\n";
    std::cout << allocateAndPrintBuffer(-5) << "\\n\\n";
    
    std::cout << "Test Case 4 (zero size):\\n";
    std::cout << allocateAndPrintBuffer(0) << "\\n\\n";
    
    std::cout << "Test Case 5 (excessive size):\\n";
    std::cout << allocateAndPrintBuffer(MAX_BUFFER_SIZE + 1) << "\\n\\n";
    
    // Command line argument handling
    if (argc > 1) {
        try {
            long long size = std::stoll(argv[1]);
            std::cout << "Command line argument:\\n";
            std::cout << allocateAndPrintBuffer(size) << "\\n";
        } catch (const std::exception& e) {
            std::cout << "Error: Invalid number format\\n";
        }
    }
    
    return 0;
}
