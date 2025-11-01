#include <iostream>
#include <vector>
#include <string>
#include <algorithm> // For std::min

/**
 * @brief Allocates a buffer of 1024 bytes.
 * @return A std::vector<char> of size 1024, initialized to zeros.
 */
std::vector<char> allocateBuffer() {
    return std::vector<char>(1024, 0);
}

/**
 * @brief Reads a line from stdin and stores the first 10 characters in the buffer.
 * @param buf The buffer to store the characters in.
 */
void readIntoBuffer(std::vector<char>& buf) {
    if (buf.size() < 10) {
        std::cerr << "Buffer is too small." << std::endl;
        return;
    }
    
    std::string line;
    std::getline(std::cin, line);
    
    // Copy up to 10 characters from the read line into the buffer
    size_t length_to_copy = std::min(line.length(), (size_t)10);
    for (size_t i = 0; i < length_to_copy; ++i) {
        buf[i] = line[i];
    }
}

int main() {
    std::cout << "Running 5 test cases for C++..." << std::endl;
    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        
        // 1. Allocate buffer
        std::vector<char> buffer = allocateBuffer();
        
        // 2. Read into buffer
        std::cout << "Enter some characters (up to 10 will be read) and press Enter: ";
        readIntoBuffer(buffer);
        
        // 3. Print the result
        std::cout << "First 10 bytes of buffer (as chars): [";
        for (int j = 0; j < 10; ++j) {
            char c = buffer[j];
            if (c < 32) { // Print non-printable chars as '.'
                std::cout << '.';
            } else {
                std::cout << c;
            }
        }
        std::cout << "]" << std::endl;
        
        std::cout << "First 10 byte values (as ints):    [";
        for (int j = 0; j < 10; ++j) {
            std::cout << static_cast<int>(buffer[j]) << (j < 9 ? ", " : "");
        }
        std::cout << "]" << std::endl;
    }
    return 0;
}