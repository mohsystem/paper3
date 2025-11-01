#include <iostream>
#include <vector>
#include <cstring> // For memcpy and strlen
#include <new>     // For std::nothrow

class Lse120 {
public:
    /**
     * Allocates two raw memory chunks, copies the content of the first
     * to the second, and then prints the second chunk.
     *
     * @param data A pointer to a null-terminated C-style string.
     */
    static void memoryCopyAndPrint(const char* data) {
        if (data == nullptr) {
            std::cerr << "Error: Input data cannot be null." << std::endl;
            return;
        }

        // Include space for the null terminator
        size_t size = strlen(data) + 1; 

        // 1. Allocate two chunks of memory using 'new'.
        // std::nothrow prevents an exception on allocation failure, returning nullptr instead.
        char* sourceChunk = new(std::nothrow) char[size];
        char* destinationChunk = new(std::nothrow) char[size];

        // Security check: Ensure memory allocation was successful.
        if (sourceChunk == nullptr || destinationChunk == nullptr) {
            std::cerr << "Error: Memory allocation failed." << std::endl;
            delete[] sourceChunk;      // Clean up if one succeeded and the other failed
            delete[] destinationChunk;
            return;
        }

        // Populate the first chunk with the input data.
        // memcpy is used for raw memory copy. It's safe here because we calculated
        // the exact size required.
        memcpy(sourceChunk, data, size);

        // 2. Copy the contents of the first chunk into the second chunk.
        memcpy(destinationChunk, sourceChunk, size);

        // 3. Print the contents of the second chunk.
        std::cout << "Contents of the second chunk: " << destinationChunk << std::endl;

        // Security best practice: Deallocate the memory to prevent memory leaks.
        delete[] sourceChunk;
        delete[] destinationChunk;
    }
};

int main() {
    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Regular string
    Lse120::memoryCopyAndPrint("Hello, C++!");

    // Test Case 2: String with numbers and symbols
    Lse120::memoryCopyAndPrint("123 Test Cases!@#");

    // Test Case 3: Longer string
    Lse120::memoryCopyAndPrint("This is a longer test string to check allocation.");

    // Test Case 4: Single character
    Lse120::memoryCopyAndPrint("A");

    // Test Case 5: Empty string
    Lse120::memoryCopyAndPrint("");

    return 0;
}