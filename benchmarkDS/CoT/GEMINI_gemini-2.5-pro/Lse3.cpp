#include <iostream>
#include <cstring> // For strlen

/**
 * Copies n bytes from memory area src to memory area dest.
 *
 * @param dest Pointer to the destination array.
 * @param src  Pointer to the source array.
 * @param n    Number of bytes to be copied.
 * @return A pointer to the destination.
 */
void* my_memcpy(void* dest, const void* src, size_t n) {
    // Security Check: Ensure pointers are not null.
    if (dest == nullptr || src == nullptr) {
        return nullptr;
    }
    
    // Cast pointers to char* to perform byte-level operations.
    char* d = static_cast<char*>(dest);
    const char* s = static_cast<const char*>(src);

    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }

    return dest;
}

int main() {
    std::cout << "CPP Tests:" << std::endl;

    // Test Case 1: Standard copy "Hello"
    const char* src1 = "Hello";
    char dest1[10] = {0}; // Zero-initialize
    size_t len1 = strlen(src1) + 1; // +1 for null terminator
    std::cout << "\n--- Test 1: Standard copy ---" << std::endl;
    std::cout << "Copying " << len1 << " bytes from \"" << src1 << "\"." << std::endl;
    std::cout << "Destination buffer size: " << sizeof(dest1) << std::endl;
    // Security check by caller: ensure copy size does not exceed destination size.
    if (len1 <= sizeof(dest1)) {
        my_memcpy(dest1, src1, len1);
        std::cout << "Destination after copy: \"" << dest1 << "\"" << std::endl;
    }

    // Test Case 2: Full buffer copy
    const char* src2 = "12345";
    char dest2[6] = {0};
    size_t len2 = strlen(src2) + 1; // 5 + 1 = 6
    std::cout << "\n--- Test 2: Full buffer copy ---" << std::endl;
    std::cout << "Copying " << len2 << " bytes from \"" << src2 << "\"." << std::endl;
    std::cout << "Destination buffer size: " << sizeof(dest2) << std::endl;
    if (len2 <= sizeof(dest2)) {
      my_memcpy(dest2, src2, len2);
      std::cout << "Destination after copy: \"" << dest2 << "\"" << std::endl;
    }

    // Test Case 3: Partial copy
    const char* src3 = "World";
    char dest3[10] = {0};
    std::cout << "\n--- Test 3: Partial copy ---" << std::endl;
    std::cout << "Copying 3 bytes from \"" << src3 << "\"." << std::endl;
    my_memcpy(dest3, src3, 3);
    std::cout << "Destination after copy: \"" << dest3 << "\" (Note: not null-terminated by the copy)" << std::endl;
    
    // Test Case 4: Copy empty string
    const char* src4 = "";
    char dest4[5] = {'X','X','X','X','\0'};
    size_t len4 = strlen(src4) + 1; // 1 byte for null terminator
    std::cout << "\n--- Test 4: Copy empty string ---" << std::endl;
    std::cout << "Destination before: \"" << dest4 << "\"" << std::endl;
    if (len4 <= sizeof(dest4)) {
        my_memcpy(dest4, src4, len4);
        std::cout << "Destination after copy: \"" << dest4 << "\"" << std::endl;
    }

    // Test Case 5: Array to array copy
    char src5[] = {'A', 'B', 'C', 'D', '\0'};
    char dest5[5] = {0};
    std::cout << "\n--- Test 5: Array to array copy ---" << std::endl;
    std::cout << "Copying " << sizeof(src5) << " bytes from a char array." << std::endl;
    if (sizeof(src5) <= sizeof(dest5)) {
        my_memcpy(dest5, src5, sizeof(src5));
        std::cout << "Destination after copy: \"" << dest5 << "\"" << std::endl;
    }
    
    return 0;
}