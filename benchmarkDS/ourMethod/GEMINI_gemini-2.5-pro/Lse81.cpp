#include <iostream>
#include <vector>
#include <string>
#include <limits>

const size_t BUFFER_SIZE = 1024;
const size_t READ_SIZE = 10;

/**
 * Allocates a buffer of BUFFER_SIZE bytes.
 * Using std::vector provides automatic memory management (RAII).
 * @return A std::vector<char> of size BUFFER_SIZE.
 */
std::vector<char> allocateBuffer() {
    return std::vector<char>(BUFFER_SIZE, 0);
}

/**
 * Reads up to READ_SIZE characters from stdin into the buffer.
 * @param buf The buffer to read into. Its size must be at least READ_SIZE.
 * @return The number of characters actually read.
 */
size_t readIntoBuffer(std::vector<char>& buf) {
    if (buf.size() < READ_SIZE) {
        std::cerr << "Buffer is too small." << std::endl;
        return 0;
    }

    std::cout << "Please enter characters. The first " << READ_SIZE << " will be read: ";
    std::cout.flush();

    // std::cin.read is a bounded read, it will not read more than READ_SIZE.
    std::cin.read(buf.data(), READ_SIZE);

    // Check how many characters were actually read.
    std::streamsize chars_read = std::cin.gcount();
    
    if (std::cin.fail() && !std::cin.eof()) {
        std::cerr << "Error reading from stdin." << std::endl;
        // Clear error flags to allow further I/O operations.
        std::cin.clear();
    }
    
    // Clear the rest of the line from the input buffer to prevent it from
    // affecting the next read in a loop.
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    return static_cast<size_t>(chars_read);
}

/**
 * Main function with 5 test cases.
 */
int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;

        std::vector<char> buffer = allocateBuffer();

        size_t bytes_read = readIntoBuffer(buffer);

        if (bytes_read > 0) {
            std::cout << "Read " << bytes_read << " characters: \"";
            // Write the read part of the buffer to stdout.
            std::cout.write(buffer.data(), bytes_read);
            std::cout << "\"" << std::endl;
        } else {
             std::cout << "No characters were read." << std::endl;
        }

        // Buffer is automatically deallocated when 'buffer' goes out of scope.
    }

    return 0;
}