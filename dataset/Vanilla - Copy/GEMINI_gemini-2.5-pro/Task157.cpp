#include <iostream>
#include <new> // Required for std::nothrow

/**
 * Demonstrates dynamic memory allocation and deallocation in C++.
 * We use 'new[]' for allocation and 'delete[]' for deallocation of arrays.
 *
 * @param size The number of integers to allocate space for.
 */
void manageMemory(int size) {
    if (size <= 0) {
        std::cout << "Cannot allocate memory for size " << size << "." << std::endl;
        return;
    }

    std::cout << "Attempting to allocate memory for an array of " << size << " integers." << std::endl;

    // 1. Dynamic Allocation
    // Using std::nothrow to prevent an exception on failure; it returns nullptr instead.
    int* dynamicArray = new (std::nothrow) int[size];

    if (dynamicArray == nullptr) {
        std::cout << "Error: Memory allocation failed." << std::endl;
        return;
    }
    
    std::cout << "Memory allocated successfully at address: " << dynamicArray << std::endl;

    // Using the allocated memory
    dynamicArray[0] = 100;
    std::cout << "First element set to: " << dynamicArray[0] << std::endl;

    // 2. Dynamic Deallocation
    std::cout << "Freeing the allocated memory." << std::endl;
    delete[] dynamicArray;
    dynamicArray = nullptr; // Good practice to nullify pointer after deletion

    std::cout << "Memory for " << size << " integers has been freed.\n" << std::endl;
}

int main() {
    std::cout << "--- C++ Memory Management Demo ---" << std::endl;
    // 5 Test Cases
    manageMemory(10);
    manageMemory(1000);
    manageMemory(50000);
    manageMemory(0);
    manageMemory(1);
    std::cout << "--- End of C++ Demo ---" << std::endl;
    return 0;
}