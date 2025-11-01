#include <iostream>
#include <cstring> // For std::memcpy and std::memset
#include <vector>   // For easy buffer management and printing

/**
 * Copies a number of bytes from one memory location to another.
 *
 * @param destination Pointer to the destination memory.
 * @param source      Pointer to the source memory.
 * @param numBytes    Number of bytes to copy.
 */
void memoryCopy(void* destination, const void* source, size_t numBytes) {
    if (destination != nullptr && source != nullptr) {
        std::memcpy(destination, source, numBytes);
    }
}

// Helper function to print a character buffer
void printBuffer(const char* label, const char* buffer, size_t size) {
    std::cout << label;
    for (size_t i = 0; i < size; ++i) {
        // Print non-printable chars as '.'
        char c = buffer[i];
        if (c >= 32 && c <= 126) {
            std::cout << c;
        } else {
            std::cout << '.';
        }
    }
    std::cout << std::endl;
}


int main() {
    // Test Case 1: Basic copy
    std::cout << "--- Test Case 1: Basic Copy ---" << std::endl;
    const char* src1 = "Hello World";
    char dest1[16];
    std::memset(dest1, 0, 16); // Clear destination buffer
    printBuffer("Destination before copy: ", dest1, 15);
    memoryCopy(dest1, src1, 5);
    printBuffer("Destination after copy:  ", dest1, 15);
    std::cout << std::endl;

    // Test Case 2: Partial copy
    std::cout << "--- Test Case 2: Partial Copy ---" << std::endl;
    const char* src2 = "Programming";
    char dest2[16];
    std::memset(dest2, 0, 16);
    printBuffer("Destination before copy: ", dest2, 15);
    memoryCopy(dest2, src2, 4);
    printBuffer("Destination after copy:  ", dest2, 15);
    std::cout << std::endl;

    // Test Case 3: Overwriting existing data
    std::cout << "--- Test Case 3: Overwriting Existing Data ---" << std::endl;
    const char* src3 = "NewData";
    char dest3[] = "OldDataOldData";
    printBuffer("Destination before copy: ", dest3, 14);
    memoryCopy(dest3, src3, 7);
    printBuffer("Destination after copy:  ", dest3, 14);
    std::cout << std::endl;

    // Test Case 4: Copying zero bytes
    std::cout << "--- Test Case 4: Copying Zero Bytes ---" << std::endl;
    const char* src4 = "Source";
    char dest4[] = "Destination";
    char original_dest4[12];
    std::memcpy(original_dest4, dest4, 12);
    printBuffer("Destination before copy: ", dest4, 11);
    memoryCopy(dest4, src4, 0);
    printBuffer("Destination after copy:  ", dest4, 11);
    std::cout << "Unchanged: " << (std::memcmp(original_dest4, dest4, 12) == 0 ? "true" : "false") << std::endl;
    std::cout << std::endl;

    // Test Case 5: Copying into a larger buffer
    std::cout << "--- Test Case 5: Copying Into a Larger Buffer ---" << std::endl;
    const char* src5 = "Short";
    char dest5[11];
    std::memset(dest5, 'X', 10);
    dest5[10] = '\0';
    printBuffer("Destination before copy: ", dest5, 10);
    memoryCopy(dest5, src5, 5);
    printBuffer("Destination after copy:  ", dest5, 10);
    std::cout << std::endl;

    return 0;
}