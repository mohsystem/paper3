
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <cerrno>

// Maximum buffer size to prevent excessive memory allocation
constexpr size_t MAX_BUFFER_SIZE = 10485760; // 10 MB

/**
 * Secure memset that cannot be optimized away by compiler
 */
void secure_zero(void* ptr, size_t len) {
    if (ptr == nullptr || len == 0) return;
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) {
        *p++ = 0;
    }
}

/**
 * Allocates and prints buffer contents
 * @param size The size of buffer to allocate
 * @return 0 on success, -1 on error
 */
int allocate_and_print_buffer(size_t size) {
    // Enforce maximum size to prevent excessive memory usage
    if (size > MAX_BUFFER_SIZE) {
        std::cerr << "Error: Buffer size exceeds maximum allowed" << std::endl;
        return -1;
    }
    
    // Allocate buffer with calloc (initializes to zero and checks overflow)
    unsigned char* buffer = static_cast<unsigned char*>(std::calloc(size, 1));
    
    // Check allocation result
    if (buffer == nullptr && size > 0) {
        std::cerr << "Error: Unable to allocate buffer" << std::endl;
        return -1;
    }
    
    // Print buffer contents (binary safe)
    if (size > 0) {
        std::cout.write(reinterpret_cast<char*>(buffer), size);
        std::cout.flush();
    }
    
    // Clear sensitive data before freeing
    secure_zero(buffer, size);
    
    // Free allocated memory
    std::free(buffer);
    buffer = nullptr;
    
    return 0;
}

int main(int argc, char* argv[]) {
    // Test cases when no arguments provided
    if (argc == 1) {
        std::cout << "Running test cases:" << std::endl;
        
        // Test 1: Normal small buffer
        std::cout << "\\nTest 1: Size 10" << std::endl;
        allocate_and_print_buffer(10);
        
        // Test 2: Zero size buffer
        std::cout << "\\nTest 2: Size 0" << std::endl;
        allocate_and_print_buffer(0);
        
        // Test 3: Moderate size buffer
        std::cout << "\\nTest 3: Size 1024" << std::endl;
        allocate_and_print_buffer(1024);
        
        // Test 4: Size at boundary
        std::cout << "\\nTest 4: Size 4096" << std::endl;
        allocate_and_print_buffer(4096);
        
        // Test 5: Excessive size (should fail)
        std::cout << "\\nTest 5: Size exceeding maximum" << std::endl;
        allocate_and_print_buffer(MAX_BUFFER_SIZE + 1);
        
        return 0;
    }
    
    // Validate argument count
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <buffer_size>" << std::endl;
        return 1;
    }
    
    // Parse and validate input
    char* endptr = nullptr;
    errno = 0;
    long long value = std::strtoll(argv[1], &endptr, 10);
    
    // Check for conversion errors
    if (endptr == argv[1] || *endptr != '\\0' || errno == ERANGE) {
        std::cerr << "Error: Invalid buffer size" << std::endl;
        return 1;
    }
    
    // Check for negative or overflow values
    if (value < 0 || value > static_cast<long long>(MAX_BUFFER_SIZE)) {
        std::cerr << "Error: Buffer size out of valid range" << std::endl;
        return 1;
    }
    
    int result = allocate_and_print_buffer(static_cast<size_t>(value));
    return (result == 0) ? 0 : 1;
}
