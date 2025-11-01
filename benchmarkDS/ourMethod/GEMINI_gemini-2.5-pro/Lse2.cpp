#include <iostream>
#include <cstring> // For memcpy, strlen
#include <vector>

/**
 * @brief Copies a number of bytes from one memory location to another.
 *
 * @warning This function is inherently unsafe as it cannot verify the size of the
 * destination buffer. The caller is solely responsible for ensuring that the
 * destination buffer `dest` is large enough to hold `n` bytes to prevent
 * buffer overflows. Secure usage requires the caller to perform bounds checks
 * before calling this function.
 *
 * @param dest Pointer to the destination memory location. Must not be null.
 * @param src Pointer to the source memory location. Must not be null.
 * @param n The number of bytes to copy.
 */
void copyBytes(void* dest, const void* src, size_t n) {
    if (dest == nullptr || src == nullptr) {
        // In a real application, log this error.
        return;
    }
    if (n == 0) {
        return;
    }
    
    // Note: memcpy has undefined behavior if src and dest overlap.
    // Use memmove for overlapping regions. Assuming non-overlapping here.
    memcpy(dest, src, n);
}

void printBuffer(const char* name, const char* buffer, size_t len) {
    std::cout << name << ": \"";
    // Ensure we don't print past the intended length
    std::cout.write(buffer, len);
    std::cout << "\"" << std::endl;
}

int main() {
    // Test Case 1: Normal successful copy
    std::cout << "--- Test Case 1: Normal Copy ---" << std::endl;
    {
        const char* src = "Hello, C++!";
        const size_t src_len_with_null = strlen(src) + 1;
        std::vector<char> dest(50, 0); // Use vector for safe memory management
        
        // Rule #1, #2: Caller performs the boundary check
        if (src_len_with_null <= dest.size()) {
            std::cout << "Attempting to copy " << src_len_with_null << " bytes." << std::endl;
            copyBytes(dest.data(), src, src_len_with_null);
            std::cout << "Copy successful." << std::endl;
            printBuffer("Destination", dest.data(), strlen(src));
        } else {
            std::cout << "Copy failed: Destination buffer too small." << std::endl;
        }
    }
    std::cout << std::endl;

    // Test Case 2: Simulating a failed copy due to insufficient destination size
    std::cout << "--- Test Case 2: Destination Too Small (Checked) ---" << std::endl;
    {
        const char* src = "This message is too long";
        const size_t src_len_with_null = strlen(src) + 1;
        std::vector<char> dest(10, 0);

        std::cout << "Attempting to copy " << src_len_with_null << " bytes into a " << dest.size() << "-byte buffer." << std::endl;
        // Rule #1, #2: The crucial safety check before calling the unsafe function
        if (src_len_with_null <= dest.size()) {
            copyBytes(dest.data(), src, src_len_with_null);
            std::cout << "Copy successful." << std::endl;
        } else {
            std::cout << "Copy aborted by caller's check, as expected." << std::endl;
        }
    }
    std::cout << std::endl;

    // Test Case 3: Copy zero bytes
    std::cout << "--- Test Case 3: Zero Bytes Copy ---" << std::endl;
    {
        const char* src = "some data";
        std::vector<char> dest(20, 'A');
        std::vector<char> original_dest = dest;
        size_t n = 0;
        
        std::cout << "Attempting to copy " << n << " bytes." << std::endl;
        copyBytes(dest.data(), src, n);
        std::cout << "Function called." << std::endl;
        if (dest == original_dest) {
            std::cout << "Destination buffer remains unchanged as expected." << std::endl;
        } else {
            std::cout << "Error: Destination buffer was modified." << std::endl;
        }
    }
    std::cout << std::endl;

    // Test Case 4: Exact size copy
    std::cout << "--- Test Case 4: Exact Size Copy ---" << std::endl;
    {
        const char* src = "exact fit";
        const size_t src_len_with_null = strlen(src) + 1;
        std::vector<char> dest(src_len_with_null, 0);
        
        if (src_len_with_null <= dest.size()) {
            std::cout << "Attempting to copy " << src_len_with_null << " bytes into a " << dest.size() << "-byte buffer." << std::endl;
            copyBytes(dest.data(), src, src_len_with_null);
            std::cout << "Copy successful." << std::endl;
            printBuffer("Destination", dest.data(), strlen(src));
        } else {
            std::cout << "Copy failed." << std::endl;
        }
    }
    std::cout << std::endl;

    // Test Case 5: Copy a partial message
    std::cout << "--- Test Case 5: Partial Copy ---" << std::endl;
    {
        const char* src = "A complete message";
        const size_t n = 8; // copy "complete"
        std::vector<char> dest(50, 0);

        if (strlen(src) >= 2 + n && n < dest.size()) {
            std::cout << "Attempting to copy " << n << " bytes from an offset." << std::endl;
            copyBytes(dest.data(), src + 2, n);
            dest[n] = '\0'; // Manually null-terminate
            std::cout << "Copy successful." << std::endl;
            printBuffer("Destination", dest.data(), n);
        } else {
            std::cout << "Copy failed due to pre-condition check." << std::endl;
        }
    }
    std::cout << std::endl;
    
    return 0;
}