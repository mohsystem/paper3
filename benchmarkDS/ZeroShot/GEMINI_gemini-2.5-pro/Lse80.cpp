#include <iostream>
#include <string>
#include <limits>
#include <cstring> // For strlen

/**
 * @brief Reads a line from stdin into a fixed-size buffer, preventing overflow.
 * 
 * This function reads characters from std::cin until a newline is found or
 * until the buffer is full (size - 1 characters). The newline character is
 * extracted from the input stream but not stored in the buffer. If the input
 * line exceeds the buffer size, the remaining characters on the line are
 * discarded to prevent them from affecting subsequent reads.
 * 
 * @param buffer A pointer to the character array to store the input.
 * @param size The total size of the buffer (e.g., 1024).
 * @return The number of characters read and stored in the buffer. Returns -1 on error.
 */
int scan(char* buffer, int size) {
    if (buffer == nullptr || size <= 0) {
        return -1; // Indicate error for invalid arguments
    }
    buffer[0] = '\0'; // Ensure buffer is empty on entry

    // Reads up to size-1 characters. It stops on newline.
    // The newline character is extracted but discarded.
    std::cin.getline(buffer, size);

    // After getline, gcount() tells us how many characters were extracted.
    // However, if the read failed because the line was too long, the failbit is set.
    if (std::cin.fail() && !std::cin.eof()) {
        // The line was longer than the buffer.
        // 1. Clear the error state on the stream.
        std::cin.clear();
        // 2. Discard the rest of the invalid input line.
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // strlen is a safe and simple way to get the number of characters
    // actually stored, as getline guarantees null-termination.
    return strlen(buffer);
}

int main() {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    const char* testPrompts[] = {
        "a short string (e.g., 'hello world'):",
        "an empty string (just press Enter):",
        "a string with exactly 1023 'a' characters:",
        "a string longer than 1023 'b' characters (e.g., 1030 'b's):",
        "a string with special characters and spaces (e.g., '  !@#$%^&*()  '):"
    };

    std::cout << "Running 5 test cases. Please provide input for each." << std::endl;

    for (int i = 0; i < 5; ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Please enter " << testPrompts[i] << std::endl;

        int charsRead = scan(buffer, BUFFER_SIZE);

        if (charsRead >= 0) {
            std::cout << "Function returned (chars read): " << charsRead << std::endl;
            std::cout << "Buffer content: \"" << buffer << "\"" << std::endl;
            std::cout << "Buffer strlen: " << strlen(buffer) << std::endl;
        } else {
            std::cerr << "Test case failed due to an error." << std::endl;
            break;
        }
    }

    return 0;
}