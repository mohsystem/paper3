#include <iostream>
#include <string>
#include <stdexcept>
#include <memory>
#include <iomanip>
#include <algorithm>
#include <limits>

// A reasonable upper limit for buffer size to prevent Denial of Service attacks
const size_t MAX_BUFFER_SIZE = 1024 * 1024; // 1MB

/**
 * @brief Allocates a buffer of a specified size and prints its contents.
 * @param size_ll The desired size of the buffer.
 */
void processBuffer(long long size_ll) {
    std::cout << "--- Processing request for buffer of size: " << size_ll << " ---" << std::endl;
    
    if (size_ll < 0) {
        std::cerr << "Error: Buffer size cannot be negative." << std::endl;
        std::cout << "--- Finished processing ---" << std::endl << std::endl;
        return;
    }
    
    if (static_cast<unsigned long long>(size_ll) > std::numeric_limits<size_t>::max()) {
        std::cerr << "Error: Requested buffer size is too large for this platform." << std::endl;
        std::cout << "--- Finished processing ---" << std::endl << std::endl;
        return;
    }

    size_t size = static_cast<size_t>(size_ll);

    if (size > MAX_BUFFER_SIZE) {
        std::cerr << "Error: Requested buffer size " << size << " exceeds maximum allowed size of " << MAX_BUFFER_SIZE << "." << std::endl;
        std::cout << "--- Finished processing ---" << std::endl << std::endl;
        return;
    }

    try {
        // Use std::unique_ptr for automatic memory management (RAII).
        // `new char[size]` allocates uninitialized memory. Reading it is undefined behavior.
        // For secure, zero-initialized memory, use `new char[size]()`.
        std::unique_ptr<char[]> buffer(new char[size]);
        
        std::cout << "Buffer of size " << size << " allocated successfully." << std::endl;
        
        // WARNING: Printing uninitialized memory is a security risk (information disclosure).
        // The output will be unpredictable garbage data.
        if (size > 0) {
            std::cout << "Buffer contents (uninitialized garbage): ";
            size_t printLimit = std::min(size, static_cast<size_t>(32));
            for (size_t i = 0; i < printLimit; ++i) {
                std::cout << std::hex << std::setw(2) << std::setfill('0') 
                          << (static_cast<int>(buffer[i]) & 0xFF) << " ";
            }
            if (size > printLimit) {
                std::cout << "...";
            }
            std::cout << std::dec << std::endl; // Switch back to decimal mode
        } else {
            std::cout << "Buffer is empty." << std::endl;
        }

    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Failed to allocate buffer of size " << size << ". " << e.what() << std::endl;
    }
    // std::unique_ptr automatically frees the memory when it goes out of scope.
    std::cout << "--- Finished processing ---" << std::endl << std::endl;
}

/**
 * @brief Main entry point. Processes command-line arguments or runs test cases.
 * @param argc Argument count.
 * @param argv Argument vector. Expects a single integer for buffer size.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char* argv[]) {
    if (argc == 2) {
        try {
            long long size = std::stoll(argv[1]);
            processBuffer(size);
        } catch (const std::invalid_argument&) {
            std::cerr << "Error: Invalid number format. Please provide an integer for the buffer size." << std::endl;
            return 1;
        } catch (const std::out_of_range&) {
            std::cerr << "Error: Number is out of range. Please provide a smaller value." << std::endl;
            return 1;
        }
    } else if (argc == 1) {
        std::cout << "No command-line argument provided. Running test cases." << std::endl;
        // 5 test cases
        processBuffer(16);          // Test Case 1: A small, valid size
        processBuffer(0);           // Test Case 2: A zero size
        processBuffer(1);           // Test Case 3: A size of 1
        processBuffer(-10);         // Test Case 4: An invalid negative size
        processBuffer(2000000);     // Test Case 5: A size that exceeds our custom limit
    } else {
        std::cerr << "Usage: " << argv[0] << " <buffer_size>" << std::endl;
        return 1;
    }

    return 0;
}