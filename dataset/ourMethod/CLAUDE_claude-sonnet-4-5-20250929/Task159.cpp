
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <cstring>

// Constants for buffer size limits
const size_t MIN_BUFFER_SIZE = 1;
const size_t MAX_BUFFER_SIZE = 1048576; // 1MB max to prevent excessive memory usage

class SecureBuffer {
private:
    std::vector<char> buffer;
    size_t buffer_size;

public:
    // Constructor with size validation
    SecureBuffer(size_t size) : buffer_size(0) {
        // Rule#3: Validate buffer size before allocation
        if (size < MIN_BUFFER_SIZE || size > MAX_BUFFER_SIZE) {
            throw std::invalid_argument("Buffer size must be between 1 and 1048576 bytes");
        }
        // Rule#3: Use RAII-compliant container for automatic memory management
        buffer.resize(size, 0);
        buffer_size = size;
    }

    // Read data from buffer at specified index with bounds checking
    bool readAt(size_t index, char& output) {
        // Rule#3: Strict bounds checking before buffer access
        if (index >= buffer_size) {
            std::cerr << "Error: Index " << index << " out of bounds (buffer size: " 
                      << buffer_size << ")" << std::endl;
            return false;
        }
        // Safe read within bounds
        output = buffer[index];
        return true;
    }

    // Write data to buffer at specified index with bounds checking
    bool writeAt(size_t index, char value) {
        // Rule#3: Strict bounds checking before buffer access
        if (index >= buffer_size) {
            std::cerr << "Error: Index " << index << " out of bounds (buffer size: " 
                      << buffer_size << ")" << std::endl;
            return false;
        }
        buffer[index] = value;
        return true;
    }

    size_t getSize() const {
        return buffer_size;
    }

    // Initialize buffer with sample data
    void initializeWithData(const std::string& data) {
        // Rule#4: Ensure copy operation respects buffer boundaries
        size_t copy_size = std::min(data.length(), buffer_size);
        std::memcpy(buffer.data(), data.c_str(), copy_size);
    }
};

// Rule#6: Validate and sanitize user input
bool getValidatedIndex(size_t max_size, size_t& index) {
    std::string input;
    std::cout << "Enter index (0-" << (max_size - 1) << "): ";
    
    if (!std::getline(std::cin, input)) {
        std::cerr << "Error: Failed to read input" << std::endl;
        return false;
    }

    // Rule#6: Validate input is not empty
    if (input.empty()) {
        std::cerr << "Error: Empty input" << std::endl;
        return false;
    }

    // Rule#6: Validate input contains only digits
    for (char c : input) {
        if (!std::isdigit(static_cast<unsigned char>(c))) {
            std::cerr << "Error: Invalid input, only digits allowed" << std::endl;
            return false;
        }
    }

    // Rule#6: Safe conversion with overflow check
    try {
        size_t value = std::stoull(input);
        if (value >= max_size) {
            std::cerr << "Error: Index must be less than " << max_size << std::endl;
            return false;
        }
        index = value;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error: Invalid number format" << std::endl;
        return false;
    }
}

int main() {
    // Test cases
    std::cout << "=== Test Case 1: Valid buffer creation and read ===" << std::endl;
    try {
        SecureBuffer buf1(100);
        buf1.initializeWithData("Hello, World! This is a secure buffer.");
        char value;
        if (buf1.readAt(7, value)) {
            std::cout << "Character at index 7: '" << value << "'" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }

    std::cout << "\\n=== Test Case 2: Out of bounds read attempt ===" << std::endl;
    try {
        SecureBuffer buf2(50);
        buf2.initializeWithData("Test buffer");
        char value;
        buf2.readAt(100, value); // Should fail bounds check
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }

    std::cout << "\\n=== Test Case 3: Boundary read (last valid index) ===" << std::endl;
    try {
        SecureBuffer buf3(20);
        buf3.initializeWithData("Boundary test data!!");
        char value;
        if (buf3.readAt(19, value)) {
            std::cout << "Character at index 19: '" << value << "'" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }

    std::cout << "\\n=== Test Case 4: Write and read back ===" << std::endl;
    try {
        SecureBuffer buf4(30);
        if (buf4.writeAt(15, 'X')) {
            char value;
            if (buf4.readAt(15, value)) {
                std::cout << "Written and read back: '" << value << "'" << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Test 4 failed: " << e.what() << std::endl;
    }

    std::cout << "\\n=== Test Case 5: Invalid buffer size ===" << std::endl;
    try {
        SecureBuffer buf5(MAX_BUFFER_SIZE + 1); // Should throw exception
    } catch (const std::exception& e) {
        std::cout << "Correctly caught exception: " << e.what() << std::endl;
    }

    return 0;
}
