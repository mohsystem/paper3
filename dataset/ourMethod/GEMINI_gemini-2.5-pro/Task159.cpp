#include <iostream>
#include <vector>
#include <stdexcept>
#include <optional>

/**
 * Creates and initializes a dynamic buffer.
 *
 * @param size The desired size of the buffer.
 * @return A std::vector<char> of the specified size.
 */
std::vector<char> create_buffer(size_t size) {
    if (size == 0) {
        throw std::invalid_argument("Buffer size must be positive.");
    }
    std::vector<char> buffer(size);
    // Populate buffer with some data
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = static_cast<char>(i * 2);
    }
    return buffer;
}

/**
 * Safely reads a character from the buffer at the given index.
 *
 * @param buffer The buffer to read from.
 * @param index The index of the character to read.
 * @return An std::optional containing the character if the index is valid, otherwise an empty optional.
 */
std::optional<char> readFromBuffer(const std::vector<char>& buffer, size_t index) {
    // Rules#1, Rules#4: Ensure that the index is within the buffer's boundaries.
    if (index < buffer.size()) {
        // Using operator[] here is safe due to the preceding bounds check.
        // Alternatively, buffer.at(index) could be used inside a try-catch block.
        return buffer[index];
    }
    return std::nullopt;
}

int main() {
    size_t bufferSize = 20;
    try {
        std::vector<char> buffer = create_buffer(bufferSize);
        std::cout << "Testing reads from a buffer of size " << bufferSize << ":" << std::endl;

        size_t testIndices[] = {0, 10, 19, static_cast<size_t>(-1), 20};

        for (int i = 0; i < 5; ++i) {
            size_t index = testIndices[i];
            std::cout << "Test Case " << (i + 1) << ": Reading at index " << index << "... ";
            
            std::optional<char> result = readFromBuffer(buffer, index);
            
            if (result.has_value()) {
                std::cout << "Success! Value: " << static_cast<int>(result.value()) << std::endl;
            } else {
                std::cout << "Failed! Index is out of bounds." << std::endl;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}