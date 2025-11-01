#include <iostream>
#include <memory>   // For std::unique_ptr
#include <limits>   // For std::numeric_limits

/**
 * @brief Allocates two 1024-byte buffers and securely reads up to 100
 * characters from stdin into each.
 */
void performAllocationAndRead() {
    // 1. Allocate first buffer using a smart pointer for safety (RAII).
    auto buf = std::make_unique<char[]>(1024);
    
    // 2. Read into first buffer.
    std::cout << "Enter up to 100 characters for the first buffer: ";
    // std::cin.getline is safe; it stops after 100 chars or a newline.
    std::cin.getline(buf.get(), 101); // 100 chars + 1 for null terminator

    // If input was > 100 chars, the stream failbit is set. We must clear it
    // and ignore the rest of the line to allow subsequent input.
    if (std::cin.fail()) {
        std::cin.clear(); // Clear error flags.
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // 3. Allocate second buffer.
    auto buf1 = std::make_unique<char[]>(1024);

    // 4. Read into second buffer.
    std::cout << "Enter up to 100 characters for the second buffer: ";
    std::cin.getline(buf1.get(), 101);

    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // Print results.
    std::cout << "\n--- Contents ---\n";
    std::cout << "First buffer: " << buf.get() << std::endl;
    std::cout << "Second buffer: " << buf1.get() << std::endl;
    std::cout << "----------------\n\n";

    // 5. Memory is freed automatically by std::unique_ptr when it goes out of scope.
}

int main() {
    for (int i = 1; i <= 5; ++i) {
        std::cout << "--- Test Case " << i << " ---\n";
        performAllocationAndRead();
    }
    return 0;
}