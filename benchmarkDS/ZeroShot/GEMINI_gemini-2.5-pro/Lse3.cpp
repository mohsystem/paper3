#include <iostream>
#include <cstring> // For std::memmove, std::strlen, std::memset
#include <vector>   // For easier buffer management in main

/**
 * Securely copies a number of bytes from one memory location to another.
 *
 * @param dest Pointer to the destination buffer.
 * @param dest_size The maximum capacity of the destination buffer.
 * @param src Pointer to the source buffer.
 * @param num_bytes The number of bytes to copy from src to dest.
 * @return true on success, false on failure.
 */
bool secure_copy_bytes(void* dest, size_t dest_size, const void* src, size_t num_bytes) {
    // 1. Check for null pointers
    if (dest == nullptr || src == nullptr) {
        std::cerr << "Error: Destination or source pointer is null." << std::endl;
        return false;
    }

    // 2. A zero-byte copy is a no-op and is considered a success.
    if (num_bytes == 0) {
        return true;
    }

    // 3. Check for buffer overflow. This is the most critical security check.
    if (num_bytes > dest_size) {
        std::cerr << "Error: Buffer overflow detected. Cannot copy " << num_bytes 
                  << " bytes into a destination of size " << dest_size << "." << std::endl;
        return false;
    }

    // 4. Use memmove for the copy. It's safer than memcpy as it correctly
    // handles potentially overlapping memory regions.
    std::memmove(dest, src, num_bytes);
    
    return true;
}

void run_tests() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    const char* source_string = "Hello";
    // +1 for the null terminator
    size_t bytes_to_copy = std::strlen(source_string) + 1;

    // Test Case 1: Normal copy
    std::cout << "\n--- Test Case 1: Normal copy ---" << std::endl;
    char dest1[10];
    std::memset(dest1, 0, sizeof(dest1)); // Clear buffer
    if (secure_copy_bytes(dest1, sizeof(dest1), source_string, bytes_to_copy)) {
        std::cout << "Success! Copied string: \"" << dest1 << "\"" << std::endl;
    } else {
        std::cout << "Failed!" << std::endl;
    }

    // Test Case 2: Exact size copy
    std::cout << "\n--- Test Case 2: Exact size copy ---" << std::endl;
    char dest2[6];
    std::memset(dest2, 0, sizeof(dest2));
    if (secure_copy_bytes(dest2, sizeof(dest2), source_string, bytes_to_copy)) {
        std::cout << "Success! Copied string: \"" << dest2 << "\"" << std::endl;
    } else {
        std::cout << "Failed!" << std::endl;
    }

    // Test Case 3: Buffer overflow attempt
    std::cout << "\n--- Test Case 3: Buffer overflow attempt ---" << std::endl;
    char dest3[4];
    std::memset(dest3, 0, sizeof(dest3));
    if (secure_copy_bytes(dest3, sizeof(dest3), source_string, bytes_to_copy)) {
        std::cout << "Success! (This should not happen)" << std::endl;
    } else {
        std::cout << "Failed as expected." << std::endl;
    }

    // Test Case 4: Null pointer
    std::cout << "\n--- Test Case 4: Null pointer ---" << std::endl;
    char* dest4 = nullptr;
    if (secure_copy_bytes(dest4, 10, source_string, bytes_to_copy)) {
        std::cout << "Success! (This should not happen)" << std::endl;
    } else {
        std::cout << "Failed as expected." << std::endl;
    }

    // Test Case 5: Zero bytes copy
    std::cout << "\n--- Test Case 5: Zero bytes copy ---" << std::endl;
    char dest5[10];
    std::memset(dest5, 0, sizeof(dest5));
    if (secure_copy_bytes(dest5, sizeof(dest5), source_string, 0)) {
        std::cout << "Success! Copied 0 bytes." << std::endl;
    } else {
        std::cout << "Failed! (This should not happen)" << std::endl;
    }
}

int main() {
    run_tests();
    return 0;
}