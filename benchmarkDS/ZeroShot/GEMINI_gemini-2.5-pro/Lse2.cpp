#include <iostream>
#include <vector>
#include <cstring> // For std::memmove and std::memset
#include <cctype>  // For isprint

/**
 * @brief Securely copies a specified number of bytes from a source to a destination buffer.
 *        This function guards against null pointers and destination buffer overflows.
 * 
 * @param dest Pointer to the destination buffer.
 * @param dest_size The total allocated size of the destination buffer.
 * @param src Pointer to the source buffer.
 * @param count The number of bytes to copy.
 * @return true if the copy was successful, false otherwise.
 */
bool secure_copy(void* dest, size_t dest_size, const void* src, size_t count) {
    // 1. Check for null pointers
    if (dest == nullptr || src == nullptr) {
        std::cerr << "Error: Source or destination pointer is null." << std::endl;
        return false;
    }

    // 2. Check for destination buffer overflow
    if (count > dest_size) {
        std::cerr << "Error: Copy would result in a buffer overflow on the destination." << std::endl;
        return false;
    }

    // 3. Trivial case: copy zero bytes, which is a success.
    if (count == 0) {
        return true;
    }

    // Use std::memmove as it's safe for overlapping memory regions
    std::memmove(dest, src, count);

    return true;
}

// Helper function to print buffers for testing
void print_buffer(const char* name, const char* buffer, size_t size) {
    std::cout << name << ": \"";
    for (size_t i = 0; i < size; ++i) {
        if (buffer[i] == '\0') break; // Stop at first null for cleaner printing
        if (isprint(static_cast<unsigned char>(buffer[i]))) {
            std::cout << buffer[i];
        } else {
            std::cout << '.';
        }
    }
    std::cout << "\"" << std::endl;
}

int main() {
    // Test Case 1: Normal successful copy
    std::cout << "--- Test Case 1: Normal Copy ---" << std::endl;
    const char* src1 = "Hello World";
    char dest1[20];
    std::memset(dest1, 0, 20);
    bool result1 = secure_copy(dest1, 20, src1, 11);
    std::cout << "Result: " << (result1 ? "Success" : "Failure") << std::endl;
    if(result1) print_buffer("Destination", dest1, 20);
    std::cout << std::endl;

    // Test Case 2: Boundary case (full buffer)
    std::cout << "--- Test Case 2: Full Buffer Copy ---" << std::endl;
    const char* src2 = "Test";
    char dest2[4];
    bool result2 = secure_copy(dest2, 4, src2, 4);
    std::cout << "Result: " << (result2 ? "Success" : "Failure") << std::endl;
    if(result2) std::cout << "Destination: \"" << std::string(dest2, 4) << "\"" << std::endl;
    std::cout << std::endl;

    // Test Case 3: Destination buffer overflow attempt
    std::cout << "--- Test Case 3: Destination Overflow Attempt ---" << std::endl;
    const char* src3 = "This is way too long";
    char dest3[5];
    std::memset(dest3, 'X', 4);
    dest3[4] = '\0';
    bool result3 = secure_copy(dest3, 5, src3, std::strlen(src3));
    std::cout << "Result: " << (result3 ? "Success" : "Failure") << std::endl;
    print_buffer("Destination (should be unchanged)", dest3, 5);
    std::cout << std::endl;

    // Test Case 4: Source over-read attempt
    // Note: This function cannot prevent source over-reads as the source size is unknown.
    // The caller is responsible for ensuring 'count' <= size of 'src'.
    // The function's security is focused on preventing the write (destination) overflow.
    std::cout << "--- Test Case 4: Valid copy (demonstrating caller responsibility) ---" << std::endl;
    const char* src4 = "short"; // length is 5
    char dest4[20];
    std::memset(dest4, 0, 20);
    bool result4 = secure_copy(dest4, 20, src4, std::strlen(src4));
    std::cout << "Note: C/C++ cannot prevent source over-reads without knowing source size." << std::endl;
    std::cout << "Result: " << (result4 ? "Success" : "Failure") << std::endl;
    if(result4) print_buffer("Destination", dest4, 20);
    std::cout << std::endl;

    // Test Case 5: Null pointer input
    std::cout << "--- Test Case 5: Null Input ---" << std::endl;
    char dest5[10];
    const char* src5 = "test";
    bool result5_a = secure_copy(nullptr, 10, src5, 4);
    std::cout << "Result (null destination): " << (result5_a ? "Success" : "Failure") << std::endl;
    bool result5_b = secure_copy(dest5, 10, nullptr, 4);
    std::cout << "Result (null source): " << (result5_b ? "Success" : "Failure") << std::endl;

    return 0;
}