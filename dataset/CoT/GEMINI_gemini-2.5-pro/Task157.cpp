#include <iostream>
#include <new> // Required for std::nothrow

/**
 * Demonstrates dynamic memory allocation and deallocation in C++.
 * We use 'new[]' for allocation and 'delete[]' for deallocation.
 * It's crucial to deallocate memory to prevent memory leaks.
 *
 * @param size The number of integers to allocate space for.
 */
void manageMemory(int size) {
    if (size <= 0) {
        std::cout << "Size must be positive. Cannot allocate memory for size: " << size << std::endl;
        return;
    }
    std::cout << "--- C++: Managing memory for size " << size << " ---" << std::endl;

    // 1. Dynamic Allocation: Using 'new[]' to allocate an array on the heap.
    // std::nothrow prevents an exception on failure, returning nullptr instead.
    int* dynamicArray = new (std::nothrow) int[size];

    // Security Check: Always check if allocation was successful.
    if (dynamicArray == nullptr) {
        std::cerr << "Error: Memory allocation failed." << std::endl;
        std::cout << "------------------------------------------\n" << std::endl;
        return;
    }
    std::cout << "Allocated an array of " << size << " integers at address: " << dynamicArray << std::endl;

    // 2. Using the allocated memory
    for (int i = 0; i < size; ++i) {
        dynamicArray[i] = i * 10;
    }
    std::cout << "Successfully used the allocated memory." << std::endl;

    // 3. Deallocation: Freeing the memory using 'delete[]'.
    delete[] dynamicArray;
    std::cout << "Memory deallocated successfully." << std::endl;

    // Security Best Practice: Set pointer to nullptr to prevent dangling pointer issues.
    dynamicArray = nullptr;
    std::cout << "Pointer set to nullptr." << std::endl;
    std::cout << "------------------------------------------\n" << std::endl;
}

int main() {
    // 5 test cases
    manageMemory(10);
    manageMemory(100);
    manageMemory(5);
    manageMemory(0); // Test case for invalid size
    manageMemory(1000);
    return 0;
}