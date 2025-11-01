#include <iostream>
#include <vector>
#include <string>
#include <cstring> // For strlen, memset, strcmp
#include <cstddef> // For size_t

/**
 * Copies a number of bytes from one memory location to another.
 * This implementation does not handle overlapping memory regions.
 * For overlapping regions, a function like memmove should be used.
 *
 * @param dest Pointer to the destination memory. Must not be null.
 * @param src  Pointer to the source memory. Must not be null.
 * @param n    The number of bytes to copy.
 * @return A pointer to the destination memory (dest), or nullptr on error.
 */
void* customMemcpy(void* dest, const void* src, size_t n) {
    // Rule #4: Input validation
    if (dest == nullptr || src == nullptr) {
        return nullptr;
    }

    // Cast pointers to char* to operate on bytes
    char* d = static_cast<char*>(dest);
    const char* s = static_cast<const char*>(src);

    // Rule #1: Ensure that buffer operations adhere to boundaries.
    // The loop iterates exactly 'n' times, copying one byte at a time.
    // The CALLER is responsible for ensuring 'dest' and 'src' point to
    // valid memory regions of at least 'n' bytes. This function cannot
    // verify the allocated sizes of the passed-in buffers.
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }

    return dest;
}

void run_test_cases() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Copy "Hello" including null terminator
    std::cout << "\n--- Test Case 1: Copy 'Hello' ---" << std::endl;
    const char* src1 = "Hello";
    char dest1[10]; // Rule #2: Ensure buffer size is sufficient
    // Copy string length + 1 for the null terminator '\0'
    size_t bytesToCopy1 = strlen(src1) + 1;
    
    std::cout << "Source: \"" << src1 << "\"" << std::endl;
    memset(dest1, 0, sizeof(dest1)); // Initialize destination buffer
    if (customMemcpy(dest1, src1, bytesToCopy1) != nullptr) {
        std::cout << "Destination (after):  \"" << dest1 << "\"" << std::endl;
        std::cout << "strcmp result: " << (strcmp(src1, dest1) == 0 ? "Match" : "Mismatch") << std::endl;
    }

    // Test Case 2: Copy with n = 0
    std::cout << "\n--- Test Case 2: Copy 0 bytes ---" << std::endl;
    const char* src2 = "Test";
    char dest2[10] = "Original";
    std::cout << "Destination (before): \"" << dest2 << "\"" << std::endl;
    customMemcpy(dest2, src2, 0);
    std::cout << "Destination (after):  \"" << dest2 << "\"" << std::endl;

    // Test Case 3: Copy part of a longer string
    std::cout << "\n--- Test Case 3: Copy part of a string ---" << std::endl;
    const char* src3 = "World12345";
    char dest3[6];
    memset(dest3, 0, sizeof(dest3));
    customMemcpy(dest3, src3, 5); // Copy "World"
    // dest3[5] is already '\0' from memset, but good practice to be explicit if it wasn't
    std::cout << "Copied part: \"" << dest3 << "\"" << std::endl;

    // Test Case 4: Copy from one buffer to another
    std::cout << "\n--- Test Case 4: Copy from vector to array ---" << std::endl;
    std::vector<char> src4 = {'A', 'B', 'C', 'D', '\0'};
    char dest4[10] = {0};
    std::cout << "Source data: \"" << src4.data() << "\"" << std::endl;
    customMemcpy(dest4, src4.data(), src4.size());
    std::cout << "Destination data: \"" << dest4 << "\"" << std::endl;
    
    // Test Case 5: Null pointer input
    std::cout << "\n--- Test Case 5: Null pointer input ---" << std::endl;
    char dest5[10];
    if (customMemcpy(nullptr, "test", 4) == nullptr) {
        std::cout << "Handled nullptr for destination correctly." << std::endl;
    }
    if (customMemcpy(dest5, nullptr, 4) == nullptr) {
        std::cout << "Handled nullptr for source correctly." << std::endl;
    }
}

int main() {
    run_test_cases();
    return 0;
}