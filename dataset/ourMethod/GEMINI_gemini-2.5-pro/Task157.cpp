#include <iostream>
#include <vector>
#include <memory> // For std::unique_ptr
#include <new>    // For std::bad_alloc

/**
 * @brief Demonstrates modern C++ dynamic allocation and deallocation using RAII.
 *
 * This function allocates an array of integers using std::make_unique, which
 * returns a smart pointer (std::unique_ptr). The memory is automatically
 * deallocated when the unique_ptr goes out of scope, preventing memory leaks.
 *
 * @param size The number of integers to allocate.
 */
void demonstrateAllocation(size_t size) {
    if (size == 0) {
        std::cout << "Allocating a zero-sized array." << std::endl;
    }
    
    std::cout << "Attempting to allocate memory for " << size << " integers..." << std::endl;

    try {
        // 1. Dynamic Allocation using a smart pointer (RAII)
        // std::make_unique is preferred as it's exception-safe
        std::unique_ptr<int[]> data = std::make_unique<int[]>(size);
        std::cout << "Successfully allocated " << (size * sizeof(int)) << " bytes." << std::endl;

        // 2. Use the allocated memory
        for (size_t i = 0; i < size; ++i) {
            data[i] = i * 2;
        }
        if (size > 0) {
            std::cout << "Used memory. First element: " << data[0] << ", Last element: " << data[size - 1] << std::endl;
        } else {
            std::cout << "Used memory. Array is empty." << std::endl;
        }

    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Memory allocation failed. " << e.what() << std::endl;
    }

    // 3. Deallocation
    // The memory is automatically freed here when 'data' (the unique_ptr) goes out of scope.
    // This is the essence of RAII (Resource Acquisition Is Initialization).
    std::cout << "unique_ptr is now out of scope. Memory has been automatically deallocated." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
}

int main() {
    std::cout << "--- C++ Dynamic Memory Demonstration ---" << std::endl;
    // Test case 1: A small, valid size
    demonstrateAllocation(10);
    // Test case 2: A larger size
    demonstrateAllocation(100000);
    // Test case 3: A zero size
    demonstrateAllocation(0);
    // Test case 4: A large size
    demonstrateAllocation(10000000);
    // Test case 5: A very large size that might fail
    // Using a large but not excessive value to avoid crashing test systems.
    // SIZE_MAX / sizeof(int) would likely be too large.
    demonstrateAllocation(1000000000); 

    return 0;
}