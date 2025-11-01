#include <iostream>
#include <cstddef> // For size_t
#include <cstring> // For strcmp and test data

class Lse1 {
public:
    /**
     * Copies a number of bytes from one memory location to another.
     * Assumes non-overlapping memory regions for safety and performance,
     * similar to the standard C memcpy.
     *
     * @param destination Pointer to the destination memory block.
     * @param source      Pointer to the source memory block.
     * @param numBytes    Number of bytes to copy.
     * @return A pointer to the destination.
     */
    static void* memoryCopy(void* destination, const void* source, size_t numBytes) {
        // 1. Security Check: Prevent Null Pointer Dereference.
        if (destination == nullptr || source == nullptr) {
            return nullptr;
        }

        // 2. Check for no-op.
        if (numBytes == 0) {
            return destination;
        }

        // 3. Cast pointers to char* to perform byte-by-byte operations.
        // Using char* is standard for byte manipulation.
        char* dest_ptr = static_cast<char*>(destination);
        const char* src_ptr = static_cast<const char*>(source);
        
        // Caller's Responsibility: The caller must ensure that source and destination
        // buffers are large enough to hold numBytes. C/C++ cannot know the
        // allocated size of a raw pointer, so bounds checking inside this function
        // is not possible. This is a primary source of vulnerabilities in C/C++.

        // 4. Core Logic: Perform the byte-by-byte copy.
        for (size_t i = 0; i < numBytes; ++i) {
            dest_ptr[i] = src_ptr[i];
        }

        return destination;
    }
};

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Normal copy
    const char* src1 = "Hello World";
    char dest1[20];
    Lse1::memoryCopy(dest1, src1, strlen(src1) + 1); // +1 for null terminator
    std::cout << "Test 1 (Normal): " << dest1 << std::endl;

    // Test Case 2: Partial copy
    const char* src2 = "Programming";
    char dest2[20] = {0}; // Initialize to zeros
    Lse1::memoryCopy(dest2, src2, 7);
    std::cout << "Test 2 (Partial): " << dest2 << std::endl;

    // Test Case 3: Zero-byte copy
    const char* src3 = "Test";
    char dest3[10] = "Original";
    Lse1::memoryCopy(dest3, src3, 0);
    std::cout << "Test 3 (Zero bytes): " << (strcmp(dest3, "Original") == 0 ? "Unchanged" : "Changed") << std::endl;

    // Test Case 4: Null input (should not crash)
    const char* src4 = "Test";
    void* result4 = Lse1::memoryCopy(nullptr, src4, 4);
    if (result4 == nullptr) {
        std::cout << "Test 4 (Null input): Handled gracefully" << std::endl;
    } else {
        std::cout << "Test 4 (Null input): Failed" << std::endl;
    }

    // Test Case 5: Copy to self (pointers are identical)
    char buffer5[] = "abcdef";
    Lse1::memoryCopy(buffer5, buffer5, 3);
    std::cout << "Test 5 (Copy to self): " << (strcmp(buffer5, "abcdef") == 0 ? "Unchanged" : "Changed") << std::endl;

    return 0;
}