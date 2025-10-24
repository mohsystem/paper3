#include <iostream>
#include <string>
#include <vector>
#include <cstring> // For strncpy

/**
 * @brief Securely copies a source string into a fixed-size destination buffer.
 *
 * This function uses strncpy to prevent buffer overflows by copying at most
 * dest_size - 1 characters from the source string. It then explicitly
 * null-terminates the destination buffer to ensure it is always a valid C-string.
 *
 * @param dest Pointer to the destination character buffer.
 * @param dest_size The total size of the destination buffer (including space for null terminator).
 * @param src Pointer to the null-terminated source string.
 */
void handleInput(char* dest, size_t dest_size, const char* src) {
    if (dest == nullptr || dest_size == 0) {
        return; // Nothing to do
    }

    // Copy at most dest_size - 1 characters. strncpy is safer than strcpy.
    strncpy(dest, src, dest_size - 1);

    // Explicitly null-terminate the buffer. strncpy does not guarantee
    // null-termination if the source string is as long as or longer than dest_size.
    dest[dest_size - 1] = '\0';
}

int main() {
    const int BUFFER_SIZE = 16;

    // Test Cases
    std::vector<std::string> testInputs = {
        "Short string",      // Case 1: Shorter than buffer
        "This is too long",  // Case 2: Longer than buffer
        "Exactly 15 char",   // Case 3: Exactly fits (15 chars + null)
        "",                  // Case 4: Empty string
        "Sixteen chars!!",   // Case 5: Source is exactly 16 chars, will be truncated
    };

    std::cout << "C++ Test Cases (Buffer Size: " << BUFFER_SIZE << ")" << std::endl;
    std::cout << "------------------------------------" << std::endl;

    for (int i = 0; i < testInputs.size(); ++i) {
        const std::string& original = testInputs[i];
        char buffer[BUFFER_SIZE];

        handleInput(buffer, BUFFER_SIZE, original.c_str());

        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "  Original:  \"" << original << "\" (length: " << original.length() << ")" << std::endl;
        std::cout << "  Processed: \"" << buffer << "\" (length: " << strlen(buffer) << ")" << std::endl << std::endl;
    }

    return 0;
}