#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>

/**
 * Allocates two chunks of memory using std::vector, copies the content 
 * from the first to the second, and prints the content of the second chunk.
 * Using std::vector ensures memory safety and automatic resource management (RAII).
 *
 * @param initialData The string data to be used for the memory chunks.
 */
void manageAndCopyMemory(const std::string& initialData) {
    try {
        // 1. Allocate and populate the first chunk of memory (source).
        // std::vector<char> safely manages a dynamic array.
        std::vector<char> source(initialData.begin(), initialData.end());
        // Add a null terminator to safely print as a C-style string.
        source.push_back('\0');

        // 2. Allocate the second chunk of memory (destination).
        // 3. Copy the contents of the first chunk into the second.
        // The copy constructor of std::vector handles allocation and copying safely.
        std::vector<char> destination = source;

        // 4. Print the contents of the second chunk.
        // Check if destination is not empty to avoid accessing data of an empty vector.
        if (!destination.empty() && destination.data() != nullptr) {
            std::cout << "Copied data: \"" << destination.data() << "\"" << std::endl;
        } else {
            // Handle the case of an empty initial string, which results in a vector with only a null terminator.
            if (!initialData.empty()) {
               std::cout << "Destination is empty or invalid." << std::endl;
            } else {
               std::cout << "Copied data: \"\"" << std::endl;
            }
        }
    } catch (const std::bad_alloc& e) {
        std::cerr << "Error: Memory allocation failed. " << e.what() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
    }
    // Memory is deallocated automatically when `source` and `destination` go out of scope.
}

int main() {
    std::cout << "--- Test Case 1: Standard string ---" << std::endl;
    manageAndCopyMemory("Hello, World!");
    
    std::cout << "\n--- Test Case 2: Empty string ---" << std::endl;
    manageAndCopyMemory("");

    std::cout << "\n--- Test Case 3: String with numbers and symbols ---" << std::endl;
    manageAndCopyMemory("123!@#$%^&*()_+");

    std::cout << "\n--- Test Case 4: Long string ---" << std::endl;
    manageAndCopyMemory("This is a much longer string to test the memory allocation and copy for a slightly larger chunk of data.");

    std::cout << "\n--- Test Case 5: Another typical string ---" << std::endl;
    manageAndCopyMemory("C++ with std::vector is safe!");
    
    return 0;
}