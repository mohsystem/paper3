#include <iostream>
#include <vector>
#include <cstring> // For std::memmove
#include <stdexcept> // For std::invalid_argument

/**
 * @brief Securely copies a number of bytes from a source to a destination buffer.
 * 
 * @param dest Pointer to the destination buffer.
 * @param dest_size The total allocated size of the destination buffer.
 * @param src Pointer to the source buffer.
 * @param n The number of bytes to copy.
 * @throws std::invalid_argument if dest or src are null.
 * @throws std::length_error if n is greater than dest_size, which would cause a buffer overflow.
 */
void secure_copy(void* dest, size_t dest_size, const void* src, size_t n) {
    if (dest == nullptr || src == nullptr) {
        throw std::invalid_argument("Source or destination pointer cannot be null.");
    }
    if (n > dest_size) {
        throw std::length_error("Destination buffer overflow: number of bytes to copy exceeds destination size.");
    }
    if (n == 0) {
        return; // Nothing to copy
    }

    // std::memmove is safe for overlapping memory regions
    std::memmove(dest, src, n);
}

// Helper function to print a vector of chars
void print_buffer(const std::string& name, const std::vector<char>& buffer) {
    std::cout << name << ": [";
    for (size_t i = 0; i < buffer.size(); ++i) {
        std::cout << static_cast<int>(buffer[i]) << (i == buffer.size() - 1 ? "" : ", ");
    }
    std::cout << "]" << std::endl;
}


int main() {
    // Test Case 1: Normal successful copy
    std::cout << "--- Test Case 1: Normal Copy ---" << std::endl;
    try {
        std::vector<char> src1 = {1, 2, 3, 4, 5, 6, 7};
        std::vector<char> dest1(5, 0);
        print_buffer("Destination before copy", dest1);
        secure_copy(dest1.data(), dest1.size(), src1.data(), 5);
        print_buffer("Destination after copy ", dest1);
        std::cout << "Expected: [1, 2, 3, 4, 5]" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Test Case 2: Destination buffer too small (potential overflow)
    std::cout << "--- Test Case 2: Destination Too Small ---" << std::endl;
    try {
        std::vector<char> src2 = {10, 20, 30, 40, 50};
        std::vector<char> dest2(3, 0);
        std::cout << "Attempting to copy 5 bytes into a 3-byte array." << std::endl;
        secure_copy(dest2.data(), dest2.size(), src2.data(), 5);
    } catch (const std::exception& e) {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Test Case 3: Copying zero bytes
    std::cout << "--- Test Case 3: Zero Bytes to Copy ---" << std::endl;
    try {
        std::vector<char> src3 = {1, 1, 1, 1, 1};
        std::vector<char> dest3 = {9, 9, 9, 9, 9};
        print_buffer("Destination before copy", dest3);
        secure_copy(dest3.data(), dest3.size(), src3.data(), 0);
        print_buffer("Destination after copy ", dest3);
        std::cout << "Expected: [9, 9, 9, 9, 9]" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Caught exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Test Case 4: Overlapping memory regions
    std::cout << "--- Test Case 4: Overlapping Memory ---" << std::endl;
    try {
        std::vector<char> buffer4 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        print_buffer("Buffer before copy", buffer4);
        char* src_ptr = buffer4.data();       // points to start
        char* dest_ptr = buffer4.data() + 2;  // points to 3rd element
        secure_copy(dest_ptr, buffer4.size() - 2, src_ptr, 5);
        print_buffer("Buffer after copy ", buffer4);
        std::cout << "Expected: [1, 2, 1, 2, 3, 4, 5, 8, 9, 10]" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Caught unexpected exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    // Test Case 5: Null pointer input
    std::cout << "--- Test Case 5: Null Input ---" << std::endl;
    try {
        std::vector<char> src5 = {1, 2, 3};
        std::cout << "Attempting to copy to a null destination." << std::endl;
        secure_copy(nullptr, 10, src5.data(), 3);
    } catch (const std::exception& e) {
        std::cerr << "Caught expected exception: " << e.what() << std::endl;
    }
    std::cout << std::endl;

    return 0;
}