#include <iostream>
#include <vector>
#include <limits> // Required for numeric_limits

// Define a class as requested by the prompt name convention
class Lse79 {
public:
    /**
     * Allocates two 1024-byte buffers, and for each, reads up to 100 characters
     * from standard input.
     */
    static void performAllocationAndRead() {
        // The first method allocates 1024 bytes of memory.
        char* buf = nullptr;
        try {
            buf = new char[1024];
        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed for buf: " << e.what() << std::endl;
            return;
        }

        // The second method allocates another 1024 bytes of memory.
        char* buf1 = nullptr;
        try {
            buf1 = new char[1024];
        } catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed for buf1: " << e.what() << std::endl;
            delete[] buf; // Clean up the first buffer
            return;
        }

        std::cout << "Enter up to 100 characters for the first buffer:" << std::endl;
        // Read up to 100 characters. Using read is safer than >>.
        std::cin.read(buf, 100);
        // gcount() returns the number of characters actually read.
        std::streamsize charsRead1 = std::cin.gcount();
        // Null-terminate the string to safely print it. This is a critical security step.
        buf[charsRead1] = '\0';

        // Clear the buffer, especially the fail state if input was less than 100 chars
        // and to ignore remaining characters on the line.
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');


        std::cout << "Enter up to 100 characters for the second buffer:" << std::endl;
        std::cin.read(buf1, 100);
        std::streamsize charsRead2 = std::cin.gcount();
        buf1[charsRead2] = '\0';
        
        // Clear the input buffer again for the next test case
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "Content of first buffer: " << buf << std::endl;
        std::cout << "Content of second buffer: " << buf1 << std::endl;

        // CRITICAL: Free the allocated memory to prevent memory leaks.
        delete[] buf;
        delete[] buf1;
    }
};

int main() {
    // Main method with 5 test cases.
    for (int i = 1; i <= 5; ++i) {
        std::cout << "\n--- Test Case " << i << " ---" << std::endl;
        // Since performAllocationAndRead consumes a line, a simple loop works.
        Lse79::performAllocationAndRead();
    }
    std::cout << "\nAll test cases finished." << std::endl;
    return 0;
}