#include <iostream>
#include <vector>
#include <string>

/**
 * Allocates a buffer of 1024 bytes using std::vector.
 * std::vector manages its own memory, preventing memory leaks.
 * @return A std::vector<char> of size 1024.
 */
std::vector<char> allocateBuffer() {
    return std::vector<char>(1024);
}

/**
 * Reads 10 characters from standard input and stores them in the buffer.
 * Using std::istream::read is safe against buffer overflows because it
 * requires specifying the maximum number of characters to read.
 * @param buf The buffer to store the read characters.
 */
void readIntoBuffer(std::vector<char>& buf) {
    if (buf.size() < 10) {
        std::cerr << "Error: Buffer is too small." << std::endl;
        return;
    }

    // Read exactly 10 characters into the beginning of the vector's data.
    std::cin.read(buf.data(), 10);
    
    // Check for read errors or if fewer than 10 characters were read (e.g., EOF).
    // gcount() returns the number of characters extracted by the last unformatted input operation.
    if (std::cin.gcount() < 10) {
        // Clear error flags if we just hit EOF, but still process what was read.
        if (std::cin.eof()) {
            std::cin.clear(); 
        } else if (std::cin.fail()) {
            std::cerr << "Error reading from input." << std::endl;
            std::cin.clear();
        }
    }
    
    // Clear the rest of the input line to prepare for the next test case.
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Test Case " << i << " ---" << std::endl;
        std::cout << "Please enter exactly 10 characters and press Enter:" << std::endl;

        std::vector<char> buffer = allocateBuffer();
        readIntoBuffer(buffer);

        std::cout << "First 10 bytes of buffer: [";
        // Safely write the first 10 characters to the output stream.
        std::cout.write(buffer.data(), 10);
        std::cout << "]" << std::endl << std::endl;
    }

    return 0;
}