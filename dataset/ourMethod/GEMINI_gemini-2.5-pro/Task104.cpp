#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstddef>

// Define a constant for the buffer size.
const size_t BUFFER_SIZE = 64;

/**
 * @brief Safely copies a string into a fixed-size buffer.
 *
 * This function takes a string and copies its contents into a fixed-size
 * character vector. If the input string is longer than the buffer, the
 * string is truncated. The buffer is always null-padded.
 *
 * @param input The std::string to copy.
 * @return A std::vector<char> representing the buffer.
 */
std::vector<char> handleInput(const std::string& input) {
    // Create a buffer of fixed size, initialized with null characters.
    std::vector<char> buffer(BUFFER_SIZE, '\0');

    // Determine the number of characters to copy to prevent overflow.
    size_t lengthToCopy = std::min(input.length(), BUFFER_SIZE);

    // Safely copy the input into the buffer. std::copy is safe here
    // because we've pre-calculated the correct length.
    if (lengthToCopy > 0) {
        std::copy_n(input.begin(), lengthToCopy, buffer.begin());
    }

    return buffer;
}

int main() {
    std::string testCases[] = {
        "This is a short input.",
        "This is a very long input string that is definitely going to be longer than the sixty-four character buffer we have defined, thus causing truncation.",
        "",
        "This input string is exactly 64 characters long to test boundary.", // 64 chars
        "Another short test."
    };

    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case #" << i + 1 << std::endl;
        std::cout << "Input: \"" << testCases[i] << "\"" << std::endl;

        std::vector<char> resultBuffer = handleInput(testCases[i]);

        // Print the content as a C-style string using the .data() pointer.
        // This is safe because the vector was initialized with nulls,
        // ensuring null-termination.
        std::cout << "Buffer Content: \"" << resultBuffer.data() << "\"" << std::endl;
        std::cout << "Buffer Size: " << resultBuffer.size() << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}