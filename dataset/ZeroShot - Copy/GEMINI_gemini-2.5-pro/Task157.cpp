#include <iostream>
#include <memory> // For std::unique_ptr
#include <new>    // For std::bad_alloc

/**
 * @brief Demonstrates secure dynamic memory allocation and deallocation in modern C++.
 *
 * This function uses a std::unique_ptr, a smart pointer that follows the
 * RAII (Resource Acquisition Is Initialization) principle. The memory is
 * allocated with `new` and automatically deallocated with `delete[]` when the
 * `unique_ptr` goes out of scope. This prevents memory leaks, even if an
 * exception occurs.
 *
 * @param size The number of integers to allocate.
 */
void demonstrateResourceManagement(size_t size) {
    if (size == 0) {
        std::cout << "Size is zero. No allocation performed." << std::endl;
        return;
    }

    try {
        // 1. Dynamic Allocation using a smart pointer for safety.
        // The memory is allocated here.
        std::unique_ptr<int[]> dynamicArray(new int[size]);
        std::cout << "Successfully allocated memory for " << size << " integers." << std::endl;

        // Use the resource...
        dynamicArray[0] = 100;
        std::cout << "First element set to: " << dynamicArray[0] << std::endl;

        // 2. Deallocation is automatic!
        // When `dynamicArray` goes out of scope at the end of this function,
        // its destructor is called, which in turn calls delete[] on the raw pointer.
        // This is secure because it's exception-safe and prevents manual memory management errors.

    } catch (const std::bad_alloc& e) {
        // Handle allocation failure. `new` throws std::bad_alloc on failure.
        std::cerr << "Memory allocation failed for size " << size << ": " << e.what() << std::endl;
    }
    std::cout << "Function scope ended. Resource automatically deallocated by unique_ptr." << std::endl;
}

int main() {
    std::cout << "--- C++ Dynamic Memory Management ---" << std::endl;
    // 5 Test Cases
    demonstrateResourceManagement(10);
    std::cout << "--------------------" << std::endl;
    demonstrateResourceManagement(1000);
    std::cout << "--------------------" << std::endl;
    demonstrateResourceManagement(0);
    std::cout << "--------------------" << std::endl;
    demonstrateResourceManagement(50000);
    std::cout << "--------------------" << std::endl;
    // A very large allocation that might fail
    demonstrateResourceManagement(1000000000);
    std::cout << "--- End of C++ Demo ---" << std::endl;
    return 0;
}