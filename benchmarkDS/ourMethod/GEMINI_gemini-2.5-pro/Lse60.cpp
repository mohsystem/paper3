#include <iostream>
#include <vector>
#include <string>
#include <cerrno>
#include <cstdlib>
#include <iomanip>

const long MAX_BUFFER_SIZE = 65536; // 64 KB limit

/**
 * @brief Allocates a buffer and prints its contents.
 * @param sizeStr The size of the buffer as a C-style string.
 */
void allocateAndPrintBuffer(const char* sizeStr) {
    if (sizeStr == nullptr) {
        std::cerr << "Error: Size argument cannot be null." << std::endl;
        return;
    }

    char* end;
    errno = 0; // Reset errno before call to strtol
    long size = std::strtol(sizeStr, &end, 10);

    // Input validation
    if (end == sizeStr || *end != '\0') {
        std::cerr << "Error: Invalid number format for size." << std::endl;
        return;
    }
    if (errno == ERANGE) {
        std::cerr << "Error: Size is out of range for a long." << std::endl;
        return;
    }
    if (size < 0) {
        std::cerr << "Error: Buffer size cannot be negative." << std::endl;
        return;
    }
    if (size > MAX_BUFFER_SIZE) {
        std::cerr << "Error: Buffer size exceeds the maximum allowed limit of " << MAX_BUFFER_SIZE << " bytes." << std::endl;
        return;
    }

    try {
        // std::vector is a safe RAII container for dynamic arrays.
        // It's value-initialized, meaning zeros for unsigned char.
        std::vector<unsigned char> buffer(size);

        std::cout << "Allocated a buffer of size: " << size << std::endl;
        std::cout << "Buffer contents (hex):" << std::endl;
        
        if (buffer.empty()) {
            std::cout << "(empty)" << std::endl;
        } else {
            for (size_t i = 0; i < buffer.size(); ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(buffer[i]) << " ";
                if ((i + 1) % 16 == 0 && i + 1 < buffer.size()) {
                    std::cout << std::endl;
                }
            }
            std::cout << std::dec << std::endl; // Reset format to decimal for subsequent output
        }

    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Failed to allocate buffer: " << e.what() << std::endl;
    }
    
    std::cout << "--------------------" << std::endl;
}

/**
 * @brief Main entry point. Uses command-line argument or runs test cases.
 */
int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <buffer_size>" << std::endl;
        std::cerr << "\nRunning test cases:" << std::endl;

        std::cout << "Test Case 1: Valid size (16)" << std::endl;
        allocateAndPrintBuffer("16");

        std::cout << "Test Case 2: Zero size" << std::endl;
        allocateAndPrintBuffer("0");

        std::cout << "Test Case 3: Negative size (-5, invalid)" << std::endl;
        allocateAndPrintBuffer("-5");

        std::cout << "Test Case 4: Non-numeric size ('abc', invalid)" << std::endl;
        allocateAndPrintBuffer("abc");

        std::cout << "Test Case 5: Size too large (" << MAX_BUFFER_SIZE + 1 << ", invalid)" << std::endl;
        std::string large_size_str = std::to_string(MAX_BUFFER_SIZE + 1);
        allocateAndPrintBuffer(large_size_str.c_str());
        
        return 1;
    }

    allocateAndPrintBuffer(argv[1]);

    return 0;
}