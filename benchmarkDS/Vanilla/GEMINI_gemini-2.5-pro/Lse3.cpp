#include <iostream>
#include <vector>
#include <cstring> // For std::strlen

/**
 * @brief Copies n bytes from memory area src to memory area dest.
 * 
 * @param dest Pointer to the destination array where the content is to be copied.
 * @param src Pointer to the source of data to be copied.
 * @param n The number of bytes to copy.
 * @return A pointer to the destination.
 */
void* memoryCopy(void* dest, const void* src, size_t n) {
    // Cast the void pointers to char pointers for byte-wise operations
    char* cdest = static_cast<char*>(dest);
    const char* csrc = static_cast<const char*>(src);

    for (size_t i = 0; i < n; ++i) {
        cdest[i] = csrc[i];
    }

    return dest;
}

void print_buffer(const char* label, const char* buffer, size_t len) {
    std::cout << label;
    for (size_t i = 0; i < len; ++i) {
        // Print non-printable characters as hex
        if (isprint(buffer[i])) {
            std::cout << buffer[i];
        } else {
            std::cout << ".";
        }
    }
    std::cout << std::endl;
}


int main() {
    // Test Case 1: Copy "Hello"
    std::cout << "--- Test Case 1: Copy 'Hello' ---" << std::endl;
    const char* src1 = "Hello";
    char dest1[10] = {0}; // Initialize with null bytes
    memoryCopy(dest1, src1, std::strlen(src1) + 1); // +1 for null terminator
    std::cout << "Source: " << src1 << std::endl;
    std::cout << "Destination: " << dest1 << std::endl;
    std::cout << std::endl;

    // Test Case 2: Copy a longer string
    std::cout << "--- Test Case 2: Copy a longer string ---" << std::endl;
    const char* src2 = "This is a test string.";
    char dest2[30] = {0};
    memoryCopy(dest2, src2, std::strlen(src2) + 1);
    std::cout << "Source: " << src2 << std::endl;
    std::cout << "Destination: " << dest2 << std::endl;
    std::cout << std::endl;

    // Test Case 3: Copy a portion of a string
    std::cout << "--- Test Case 3: Copy a portion ---" << std::endl;
    const char* src3 = "Programming";
    char dest3[15] = {0};
    memoryCopy(dest3, src3, 7); // Copy "Program", dest3 is not null-terminated by the copy
    dest3[7] = '\0'; // Manually add null terminator
    std::cout << "Source: " << src3 << std::endl;
    std::cout << "Destination (first 7 bytes): " << dest3 << std::endl;
    std::cout << std::endl;
    
    // Test Case 4: Copy an array of integers
    std::cout << "--- Test Case 4: Copy integer array ---" << std::endl;
    int src4[] = {100, 200, 300, 400};
    int dest4[4] = {0};
    memoryCopy(dest4, src4, sizeof(src4));
    std::cout << "Source integers: ";
    for(int i = 0; i < 4; ++i) std::cout << src4[i] << " ";
    std::cout << std::endl;
    std::cout << "Destination integers: ";
    for(int i = 0; i < 4; ++i) std::cout << dest4[i] << " ";
    std::cout << std::endl << std::endl;

    // Test Case 5: Copy zero bytes
    std::cout << "--- Test Case 5: Copy zero bytes ---" << std::endl;
    const char* src5 = "Should not be copied";
    char dest5[20];
    // Fill destination to see the effect
    for(int i = 0; i < 19; ++i) dest5[i] = 'X';
    dest5[19] = '\0';
    memoryCopy(dest5, src5, 0);
    std::cout << "Source: " << src5 << std::endl;
    std::cout << "Destination after copying 0 bytes: " << dest5 << std::endl;
    std::cout << std::endl;

    return 0;
}