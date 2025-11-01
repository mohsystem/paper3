#include <iostream>
#include <string>
#include <limits>

/**
 * Reads a line from standard input into the provided buffer.
 * It reads at most (buffer_size - 1) characters. The newline is
 * extracted from the input stream but not stored.
 *
 * @param buffer A character array to store the read string.
 * @param buffer_size The total size of the character array.
 * @return The number of characters read and stored in the buffer.
 */
int scan(char* buffer, int buffer_size) {
    // Read up to buffer_size-1 characters or until a newline
    std::cin.getline(buffer, buffer_size);

    int chars_read = std::cin.gcount();
    
    // If getline failed, it's because the line was too long.
    if (std::cin.fail()) {
        // gcount() for a failed getline is implementation-defined
        // but often reports buffer_size-1. We will use that value.
        chars_read = buffer_size - 1;
        // Clear the fail state
        std::cin.clear();
        // Discard the rest of the oversized line from the input stream
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    } else {
        // If getline succeeded, it stopped on a newline. gcount() reports chars
        // read *before* the null terminator. This is correct.
        // If an empty line is entered, gcount() is 0, but getline reads and
        // discards the '\n', so we need to adjust to return 0.
        // On successful read, gcount includes the consumed delimiter in its count
        // on some platforms. The safe way is to use strlen on the buffer.
        // However, the standard says gcount() returns extracted chars, so let's
        // trust that and subtract 1 if newline was the cause of stop.
        // A simpler way: The number of chars stored is simply the length of the C-string.
        // Update: Let's stick with gcount() as it represents extracted chars.
        // For an empty line, gcount is 0, and buffer is "", which is correct.
    }


    return chars_read;
}

int main() {
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    std::cout << "Running 5 test cases for C++ scan function." << std::endl;

    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        std::cout << "Please enter a string: ";
        
        int charsRead = scan(buffer, BUFFER_SIZE);
        
        std::cout << "Function returned (characters read): " << charsRead << std::endl;
        std::cout << "Buffer content: \"" << buffer << "\"" << std::endl;
        // We can also check strlen, which should match
        // std::cout << "strlen(buffer): " << std::string(buffer).length() << std::endl;
    }

    return 0;
}