#include <iostream>
#include <vector>
#include <cstring> // For test comparison with std::memcpy

// The main class as requested by the prompt
class Lse2 {
public:
    /**
     * @brief Copies n bytes from memory area src to memory area dest.
     * @param dest A pointer to the destination memory area.
     * @param src A pointer to the source memory area.
     * @param n The number of bytes to be copied.
     * @note The memory areas must not overlap.
     */
    static void my_memcpy(void* dest, const void* src, size_t n) {
        // 1. Security Check: Handle null pointers to prevent segmentation faults.
        if (dest == nullptr || src == nullptr) {
            std::cerr << "Error: Destination or source pointer is null." << std::endl;
            return;
        }

        // Cast void pointers to char pointers for byte-level manipulation.
        // char is guaranteed to be 1 byte.
        char* d = static_cast<char*>(dest);
        const char* s = static_cast<const char*>(src);
        
        // 2. Perform the copy operation.
        // The loop is a secure way to copy as long as the initial checks on 'n'
        // (done by the caller) ensure it doesn't exceed buffer boundaries.
        for (size_t i = 0; i < n; ++i) {
            d[i] = s[i];
        }
    }

    static void run_tests() {
        // --- Test Cases ---

        // Test Case 1: Standard copy of a string.
        std::cout << "--- Test Case 1: Standard copy ---" << std::endl;
        const char* src1 = "Hello C++";
        char dest1[20] = {0}; // Initialize with zeros
        size_t n1 = strlen(src1) + 1; // +1 for null terminator
        std::cout << "Before copy: dest1 = \"" << dest1 << "\"" << std::endl;
        my_memcpy(dest1, src1, n1);
        std::cout << "After copy:  dest1 = \"" << dest1 << "\"" << std::endl;
        std::cout << std::endl;

        // Test Case 2: Partial copy of an integer array.
        std::cout << "--- Test Case 2: Partial copy ---" << std::endl;
        int src2[] = {100, 200, 300, 400, 500};
        int dest2[10] = {0};
        size_t n2 = 3 * sizeof(int); // Copy first 3 integers
        std::cout << "Before copy: dest2[0] = " << dest2[0] << ", dest2[2] = " << dest2[2] << std::endl;
        my_memcpy(dest2, src2, n2);
        std::cout << "After copy:  dest2[0] = " << dest2[0] << ", dest2[2] = " << dest2[2] << std::endl;
        std::cout << std::endl;

        // Test Case 3: Zero-byte copy.
        std::cout << "--- Test Case 3: Zero-byte copy ---" << std::endl;
        const char* src3 = "Source";
        char dest3[10] = "Original";
        size_t n3 = 0;
        std::cout << "Before copy: dest3 = \"" << dest3 << "\"" << std::endl;
        my_memcpy(dest3, src3, n3);
        std::cout << "After copy:  dest3 = \"" << dest3 << "\"" << std::endl;
        std::cout << std::endl;

        // Test Case 4: Copying into a buffer that is smaller than the source, but copy size is safe.
        std::cout << "--- Test Case 4: Safe copy to smaller buffer ---" << std::endl;
        const char* src4 = "This is a long source string";
        char dest4[6] = {0};
        size_t n4 = 5;
        std::cout << "Before copy: dest4 = \"" << dest4 << "\"" << std::endl;
        my_memcpy(dest4, src4, n4); // dest4 will not be null-terminated automatically
        dest4[5] = '\0'; // Manually add null terminator for printing
        std::cout << "After copy:  dest4 = \"" << dest4 << "\"" << std::endl;
        std::cout << std::endl;
        
        // Test Case 5: Boundary Check using vectors (demonstrates a common C++ use case).
        // The check against overflow must be done *before* calling the function.
        std::cout << "--- Test Case 5: Boundary check (caller's responsibility) ---" << std::endl;
        std::vector<char> src5 = {'a', 'b', 'c', 'd', 'e'};
        std::vector<char> dest5(3, 'z');
        size_t n5 = 5; // This size would cause an overflow
        std::cout << "Attempting to copy " << n5 << " bytes into a 3-byte buffer." << std::endl;
        if (n5 > dest5.size()) {
            std::cout << "Error: Copy size " << n5 << " is larger than destination buffer size " << dest5.size() << ". Aborting copy." << std::endl;
        } else {
             my_memcpy(dest5.data(), src5.data(), n5);
        }
        std::cout << "Final state of dest5[0]: " << dest5[0] << std::endl;
        std::cout << std::endl;
    }
};

int main() {
    Lse2::run_tests();
    return 0;
}