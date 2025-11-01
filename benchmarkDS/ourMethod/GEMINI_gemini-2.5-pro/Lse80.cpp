#include <iostream>
#include <string>
#include <limits>
#include <vector>

const int BUFFER_SIZE = 1024;
const int MAX_CHARS = BUFFER_SIZE - 1;

/**
 * Reads a line from standard input into a provided buffer.
 * Reads up to MAX_CHARS characters or until a newline is found.
 * The newline character is extracted from the input stream but not stored.
 * If the input line is longer than the buffer can hold, the rest of the
 * line is discarded to prevent it from affecting subsequent reads.
 *
 * @param buffer A character array to store the read string.
 * @param size The total size of the buffer.
 * @return The number of characters read and stored in the buffer. Returns 0 if no
 *         characters were read or an error occurred.
 */
int scan(char* buffer, std::streamsize size) {
    if (buffer == nullptr || size <= 0) {
        return 0;
    }

    // Read a line from stdin, up to size-1 characters
    std::cin.getline(buffer, size);

    std::streamsize chars_read = std::cin.gcount();
    
    // getline sets failbit if it stops because it read size-1 characters
    // without seeing a newline. We need to handle this case.
    if (std::cin.fail()) {
        // Clear the fail state
        std::cin.clear();
        
        // Discard the rest of the oversized line
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    // gcount() returns the number of characters extracted, which is the length
    // of the string in the buffer.
    return static_cast<int>(chars_read);
}


int main() {
    char buffer[BUFFER_SIZE];
    
    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::cout << "\nTest Case " << i << ":" << std::endl;
        std::cout << "Please enter a line of text (or press Ctrl+D/Ctrl+Z for EOF):" << std::endl;
        
        // Check for EOF before calling scan
        if (std::cin.peek() == EOF && std::cin.eof()) {
            std::cout << "End of input reached." << std::endl;
            break;
        }
        
        int chars_read = scan(buffer, BUFFER_SIZE);

        if (chars_read > 0 || !std::cin.eof()) {
            std::cout << "String read: \"" << buffer << "\"" << std::endl;
            std::cout << "Characters read: " << chars_read << std::endl;
        } else {
             std::cout << "End of input reached." << std::endl;
             break;
        }
    }
    std::cout << "\n--- Tests Complete ---" << std::endl;

    return 0;
}