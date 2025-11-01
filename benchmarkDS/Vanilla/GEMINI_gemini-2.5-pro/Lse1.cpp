#include <iostream>
#include <cstddef> // For size_t

/**
 * Copies n bytes from the memory area pointed to by source to the
 * memory area pointed to by destination.
 *
 * @param destination Pointer to the destination memory.
 * @param source      Pointer to the source memory.
 * @param n           Number of bytes to copy.
 * @return            A pointer to the destination.
 */
void* copyBytes(void* destination, const void* source, size_t n) {
    char* dest_char = static_cast<char*>(destination);
    const char* src_char = static_cast<const char*>(source);

    for (size_t i = 0; i < n; ++i) {
        dest_char[i] = src_char[i];
    }
    return destination;
}

// Helper function to print an array of chars
void print_array(const char* label, const char* arr, size_t size) {
    std::cout << label;
    for (size_t i = 0; i < size; ++i) {
        // Cast to int to print numeric value instead of character
        std::cout << static_cast<int>(arr[i]) << " ";
    }
    std::cout << std::endl;
}

int main() {
    // Test Case 1: Basic full copy
    std::cout << "--- Test Case 1: Basic full copy ---" << std::endl;
    char src1[] = {1, 2, 3, 4, 5};
    char dest1[5] = {0};
    print_array("Before: dest1 = ", dest1, 5);
    copyBytes(dest1, src1, 5);
    print_array("After:  dest1 = ", dest1, 5);
    std::cout << std::endl;

    // Test Case 2: Partial copy
    std::cout << "--- Test Case 2: Partial copy ---" << std::endl;
    char src2[] = {10, 20, 30, 40, 50};
    char dest2[] = {99, 98, 97, 96, 95};
    print_array("Before: dest2 = ", dest2, 5);
    copyBytes(dest2, src2, 3);
    print_array("After:  dest2 = ", dest2, 5);
    std::cout << std::endl;

    // Test Case 3: Zero-length copy
    std::cout << "--- Test Case 3: Zero-length copy ---" << std::endl;
    char src3[] = {1, 1, 1};
    char dest3[] = {2, 2, 2};
    print_array("Before: dest3 = ", dest3, 3);
    copyBytes(dest3, src3, 0);
    print_array("After:  dest3 = ", dest3, 3);
    std::cout << std::endl;

    // Test Case 4: Full overwrite of a smaller source into a larger destination
    std::cout << "--- Test Case 4: Overwriting a larger destination ---" << std::endl;
    char src4[] = {7, 8};
    char dest4[] = {1, 2, 3, 4};
    print_array("Before: dest4 = ", dest4, 4);
    copyBytes(dest4, src4, 2);
    print_array("After:  dest4 = ", dest4, 4);
    std::cout << std::endl;

    // Test Case 5: Copying into a larger buffer
    std::cout << "--- Test Case 5: Copying into a larger buffer ---" << std::endl;
    char src5[] = {1, 2, 3};
    char dest5[6] = {0};
    print_array("Before: dest5 = ", dest5, 6);
    copyBytes(dest5, src5, 3);
    print_array("After:  dest5 = ", dest5, 6);
    std::cout << std::endl;

    return 0;
}