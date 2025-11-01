#include <iostream>
#include <cstddef> // For size_t
#include <vector>

void demonstrateVulnerability(size_t buffer_size) {
    if (buffer_size == 0) {
        std::cout << "Buffer size cannot be zero." << std::endl;
        return;
    }
    std::cout << "\n--- Demonstrating double free with buffer size: " << buffer_size << " ---" << std::endl;

    // 1. Allocate the buffer. Using new[] for an array.
    char* buffer = new (std::nothrow) char[buffer_size];
    if (buffer == nullptr) {
        std::cerr << "Failed to allocate memory." << std::endl;
        return;
    }

    std::cout << "Buffer allocated at: " << static_cast<void*>(buffer) << std::endl;

    // 2. DANGEROUS: Free the buffer in a loop.
    // This is a classic Double Free (CWE-415) vulnerability.
    // The first call to delete[] is correct. The second call is a double free,
    // which leads to undefined behavior and will likely crash the program.
    // This code is for educational purposes only.
    for (int i = 0; i < 2; ++i) {
        std::cout << "Loop " << i + 1 << ": Attempting to free buffer..." << std::endl;
        delete[] buffer;
        // This line may not be reached on the second iteration due to a crash.
        std::cout << "Buffer freed (iteration " << i + 1 << ")." << std::endl;
    }
    
    // In modern C++, raw memory management should be avoided.
    // Use containers like std::vector or smart pointers like std::unique_ptr
    // to manage memory automatically and prevent such errors.
    
    std::cout << "Demonstration finished. If you see this, the program did not crash as expected." << std::endl;
}

int main() {
    std::cout << "--- C++ Double Free Demonstration ---" << std::endl;
    std::cout << "WARNING: This program demonstrates a critical memory error and is expected to crash." << std::endl;

    std::vector<size_t> test_cases = {16, 32, 64, 128, 256};
    
    for (size_t i = 0; i < test_cases.size(); ++i) {
        std::cout << "\n--- Test Case " << i + 1 << " ---";
        demonstrateVulnerability(test_cases[i]);
        if (i == 0) {
            std::cout << "\nNOTE: The program likely crashed on the first test case." << std::endl;
            std::cout << "If it continues, the behavior is still undefined and highly insecure." << std::endl;
        }
    }
    
    return 0;
}