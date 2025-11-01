#include <iostream>
#include <vector>
#include <string>
#include <limits>

const int BUFFER_SIZE = 1024;
const int MAX_READ_SIZE = 100;

/**
 * @brief Allocates two buffers and reads user input into them safely.
 *
 * This function allocates two buffers of BUFFER_SIZE bytes each. It then prompts
 * the user to enter up to MAX_READ_SIZE characters from standard input for each
 * buffer, ensuring no buffer overflows occur.
 */
void allocateAndRead() {
    // Use std::vector for automatic memory management (RAII)
    std::vector<char> buf(BUFFER_SIZE, 0); // Zero-initialized
    std::cout << "Enter up to " << MAX_READ_SIZE << " characters for the first buffer:" << std::endl;

    // Read up to MAX_READ_SIZE characters + null terminator
    if (!std::cin.getline(buf.data(), MAX_READ_SIZE + 1)) {
        if (std::cin.eof()) {
            std::cout << "End of input reached." << std::endl;
            return;
        }
        // If the read fails (e.g., input is too long), clear the error state
        // and discard the rest of the line to prevent issues with the next read.
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    // Using buf.data() is safe as std::vector guarantees contiguous memory.
    std::cout << "First buffer content: " << buf.data() << std::endl;

    // Allocate the second buffer
    std::vector<char> buf1(BUFFER_SIZE, 0);
    std::cout << "Enter up to " << MAX_READ_SIZE << " characters for the second buffer:" << std::endl;

    if (!std::cin.getline(buf1.data(), MAX_READ_SIZE + 1)) {
        if (std::cin.eof()) {
            std::cout << "End of input reached." << std::endl;
            return;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
    
    std::cout << "Second buffer content: " << buf1.data() << std::endl;
}

int main() {
    // In an interactive program, test cases are run by providing different inputs.
    // The loop structure is commented out to show how 5 tests could be structured,
    // but for a single execution, running the function once is standard.
    
    std::cout << "--- Test Case 1 ---" << std::endl;
    allocateAndRead();
    
    // std::cout << "\n--- Test Case 2 ---" << std::endl;
    // allocateAndRead();
    
    // std::cout << "\n--- Test Case 3 ---" << std::endl;
    // allocateAndRead();
    
    // std::cout << "\n--- Test Case 4 ---" << std::endl;
    // allocateAndRead();
    
    // std::cout << "\n--- Test Case 5 ---" << std::endl;
    // allocateAndRead();

    return 0;
}