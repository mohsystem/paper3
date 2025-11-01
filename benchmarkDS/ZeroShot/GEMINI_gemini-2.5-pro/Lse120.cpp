#include <iostream>
#include <string>
#include <vector>
#include <memory>   // For std::unique_ptr for secure memory management
#include <cstring>  // For memcpy

class Lse120 {
public:
    /**
     * @brief Allocates two memory chunks, securely copies content between them.
     * 
     * This function uses std::unique_ptr for automatic and safe memory management (RAII),
     * preventing memory leaks. The copy operation's size is explicitly controlled
     * to prevent buffer overflows.
     * 
     * @param content The string content to copy.
     * @return A new std::string with the copied content.
     */
    static std::string secureMemoryCopy(const std::string& content) {
        size_t size = content.length();
        // Allocate space for the string content plus the null terminator
        size_t bufferSize = size + 1;

        // 1. Allocate two chunks of memory using smart pointers.
        // std::unique_ptr ensures that memory is automatically freed when the
        // function exits, preventing memory leaks. This is exception-safe.
        std::unique_ptr<char[]> source = std::make_unique<char[]>(bufferSize);
        std::unique_ptr<char[]> destination = std::make_unique<char[]>(bufferSize);

        // Copy original content into the first allocated chunk.
        // This is safe because 'source' is allocated with 'bufferSize'.
        memcpy(source.get(), content.c_str(), bufferSize);

        // 2. Securely copy the contents of the first chunk into the second chunk.
        // This copy is secure because:
        // a) We allocated 'destination' with enough space (bufferSize).
        // b) We are copying exactly 'bufferSize' bytes, which does not overflow.
        memcpy(destination.get(), source.get(), bufferSize);
        
        // 3. Create a string from the second chunk to be returned.
        // The unique_ptrs for 'source' and 'destination' automatically free their
        // memory here as they go out of scope.
        return std::string(destination.get());
    }
};

int main() {
    std::vector<std::string> testCases = {
        "Hello, secure world!",
        "This is a test case with various characters 1@#$%.",
        "Short",
        "", // Empty string test case
        "This is a significantly longer string to ensure that the memory allocation and copying process works correctly with a larger amount of data."
    };

    int i = 1;
    for (const auto& test_str : testCases) {
        std::cout << "Test Case " << i++ << ":" << std::endl;
        std::cout << "Original:    \"" << test_str << "\"" << std::endl;
        
        std::string copiedContent = Lse120::secureMemoryCopy(test_str);
        
        std::cout << "Copied:      \"" << copiedContent << "\"" << std::endl;
        std::cout << "Content matches: " << (test_str == copiedContent ? "true" : "false") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}