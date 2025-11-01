#include <iostream>
#include <string>
#include <cstring> // For memcpy
#include <new>     // For std::nothrow

/**
 * @brief Allocates two memory chunks, copies data, and demonstrates memory management.
 *
 * This function allocates two dynamic char arrays. It copies the content of the
 * provided string into the first, then copies the first chunk to the second.
 * It prints the second chunk's content before and after freeing the first chunk
 * to show they are independent. Finally, it frees the second chunk.
 *
 * @param initialData The string to initialize the first memory chunk with.
 */
void demonstrateMemoryOperations(const std::string& initialData) {
    std::cout << "--- Demonstrating with data: \"" << initialData << "\" ---" << std::endl;

    // Use the length of the string + 1 for the null terminator.
    size_t size = initialData.length() + 1;

    // 1. Allocate two chunks of memory.
    // Using `new (std::nothrow)` to prevent exceptions on allocation failure.
    char* firstChunk = new (std::nothrow) char[size];
    if (!firstChunk) {
        std::cerr << "Error: Memory allocation failed for firstChunk." << std::endl;
        return;
    }

    char* secondChunk = new (std::nothrow) char[size];
    if (!secondChunk) {
        std::cerr << "Error: Memory allocation failed for secondChunk." << std::endl;
        delete[] firstChunk; // Clean up the first allocation
        return;
    }

    // Initialize the first chunk with the data.
    // memcpy is used for raw memory copy. It's safe here because we calculated the exact size needed.
    memcpy(firstChunk, initialData.c_str(), size);

    // 2. Copy the contents of the first chunk into the second chunk.
    memcpy(secondChunk, firstChunk, size);

    // 3. Print the contents of the second chunk.
    std::cout << "Content of second chunk after copy: " << secondChunk << std::endl;

    // 4. Free the memory allocated to the first chunk.
    delete[] firstChunk;
    firstChunk = nullptr; // Good practice to nullify pointer after delete.
    std::cout << "First chunk freed." << std::endl;

    // 5. Print the contents of the second chunk again.
    // The content remains because the second chunk is a separate memory area.
    std::cout << "Content of second chunk after freeing first: " << secondChunk << std::endl;

    // 6. Free the memory allocated to the second chunk.
    delete[] secondChunk;
    secondChunk = nullptr;
    std::cout << "Second chunk freed." << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;
}

int main() {
    // 5 Test Cases
    demonstrateMemoryOperations("Test case 1: Hello, C++!");
    demonstrateMemoryOperations("Test case 2: A different string.");
    demonstrateMemoryOperations("Test case 3: Short");
    demonstrateMemoryOperations("Test case 4: A much longer string to test allocation with different sizes.");
    demonstrateMemoryOperations("Test case 5: Final test case with symbols!@#$%");

    return 0;
}