#include <iostream>
#include <cstring> // For memcpy
#include <vector>   // For test cases

class Lse120 {
public:
    static void allocateCopyAndPrint(const void* sourceData, size_t size) {
        if (sourceData == nullptr && size > 0) {
            std::cout << "Error: Source data is null but size is non-zero." << std::endl;
            return;
        }
        
        // 1. Allocate the first chunk of memory on the heap.
        char* firstChunk = new(std::nothrow) char[size];
        if (!firstChunk) {
            std::cerr << "Memory allocation failed for first chunk." << std::endl;
            return;
        }

        // Copy source data into the first chunk.
        if (size > 0) {
            memcpy(firstChunk, sourceData, size);
        }

        // 2. Allocate the second chunk of memory on the heap.
        char* secondChunk = new(std::nothrow) char[size];
        if (!secondChunk) {
            std::cerr << "Memory allocation failed for second chunk." << std::endl;
            delete[] firstChunk; // Clean up the first chunk
            return;
        }

        // 3. Copy the contents of the first chunk into the second chunk.
        if (size > 0) {
            memcpy(secondChunk, firstChunk, size);
        }
        
        // 4. Print the contents of the second chunk.
        std::cout << "Contents of the second chunk: [";
        for (size_t i = 0; i < size; ++i) {
            // Cast to int to print numerical value instead of a character
            std::cout << static_cast<int>(secondChunk[i]);
            if (i < size - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;

        // 5. Free the allocated memory to prevent leaks.
        delete[] firstChunk;
        delete[] secondChunk;
    }
};

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Regular char array
    std::cout << "\nTest Case 1:" << std::endl;
    char data1[] = {10, 20, 30, 40, 50};
    Lse120::allocateCopyAndPrint(data1, sizeof(data1));

    // Test Case 2: C-style string
    std::cout << "\nTest Case 2:" << std::endl;
    const char* data2 = "Hello";
    Lse120::allocateCopyAndPrint(data2, strlen(data2));

    // Test Case 3: Empty array
    std::cout << "\nTest Case 3:" << std::endl;
    char data3[] = {};
    Lse120::allocateCopyAndPrint(data3, sizeof(data3));

    // Test Case 4: Single element array
    std::cout << "\nTest Case 4:" << std::endl;
    char data4[] = {127};
    Lse120::allocateCopyAndPrint(data4, sizeof(data4));

    // Test Case 5: Array with negative values
    std::cout << "\nTest Case 5:" << std::endl;
    char data5[] = {-1, -128, 0, 1, 2};
    Lse120::allocateCopyAndPrint(data5, sizeof(data5));
    
    return 0;
}