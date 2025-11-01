#include <iostream>
#include <vector>
#include <new>

/**
 * Allocates a buffer of a specified size and prints its contents.
 * A std::vector is value-initialized, which means zeros for char types.
 *
 * @param size The size of the buffer to allocate.
 */
void allocateAndPrintBuffer(int size) {
    std::cout << "--- Testing with buffer size: " << size << " ---" << std::endl;
    if (size < 0) {
        std::cout << "Error: Size cannot be negative." << std::endl << std::endl;
        return;
    }

    try {
        // Allocate a buffer of the specified size using std::vector.
        std::vector<char> buffer(size);

        // Print the contents of the buffer.
        std::cout << "Allocated buffer contents: [";
        for (size_t i = 0; i < buffer.size(); ++i) {
            // Cast to int to print the numeric value '0' instead of the NUL character.
            std::cout << static_cast<int>(buffer[i]);
            if (i < buffer.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << "]" << std::endl;
    } catch (const std::bad_alloc& e) {
        std::cout << "Error: Could not allocate buffer of size " << size << ". " << e.what() << std::endl;
    }
    std::cout << std::endl;
}

/**
 * Main function with 5 test cases.
 */
int main() {
    int test_sizes[] = {10, 0, 1, 8, 5};
    for (int size : test_sizes) {
        allocateAndPrintBuffer(size);
    }
    return 0;
}