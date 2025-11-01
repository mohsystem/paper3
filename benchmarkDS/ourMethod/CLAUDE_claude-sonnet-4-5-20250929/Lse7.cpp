
#include <iostream>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <cstring>

int main(int argc, char* argv[]) {
    try {
        // Validate that we have at least 2 arguments
        if (argc < 3 || argv == nullptr) {
            std::cerr << "Error: Insufficient arguments" << std::endl;
            return 1;
        }
        
        // Validate argv[2] is not null
        if (argv[2] == nullptr) {
            std::cerr << "Error: Invalid index" << std::endl;
            return 1;
        }
        
        // Validate the string is not empty
        size_t len = std::strlen(argv[2]);
        if (len == 0 || len > 10) { // Reasonable limit for integer string
            std::cerr << "Error: Invalid index format" << std::endl;
            return 1;
        }
        
        // Convert string to integer with validation
        char* endptr = nullptr;
        errno = 0;
        long index_long = std::strtol(argv[2], &endptr, 10);
        
        // Check for conversion errors
        if (errno == ERANGE || endptr == argv[2] || *endptr != '\\0') {
            std::cerr << "Error: Invalid number format" << std::endl;
            return 1;
        }
        
        // Check for integer overflow when converting long to int
        if (index_long < 0 || index_long > INT_MAX) {
            std::cerr << "Error: Index out of range" << std::endl;
            return 1;
        }
        
        int index = static_cast<int>(index_long);
        
        // Validate index is within valid range [0, argc-1]
        // Prevent out of bounds access
        if (index >= argc) {
            std::cerr << "Error: Index out of bounds" << std::endl;
            return 1;
        }
        
        // Safely print the argument at the validated index
        std::cout << argv[index] << std::endl;
        
    } catch (const std::exception& e) {
        // Catch any unexpected exceptions, log generically
        std::cerr << "Error: Operation failed" << std::endl;
        return 1;
    }
    
    return 0;
}
