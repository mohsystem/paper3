#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <limits>

static constexpr std::size_t MAX_SIZE = 1u << 20; // 1 MiB cap

// Allocates two chunks, copies first to second, prints second, frees first, prints second, frees second.
// Returns a copy of the second chunk content captured before freeing it.
std::string demoCopy(const std::string& input) {
    if (input.size() > MAX_SIZE) {
        std::cout << "Input too large" << std::endl;
        return std::string();
    }

    const std::size_t len = input.size();

    // Allocate two chunks (+1 for null terminator)
    std::unique_ptr<char[]> chunk1(new (std::nothrow) char[len + 1]);
    std::unique_ptr<char[]> chunk2(new (std::nothrow) char[len + 1]);
    if (!chunk1 || !chunk2) {
        std::cout << "Allocation error" << std::endl;
        return std::string();
    }

    // Copy input -> chunk1 (bounded)
    if (len > 0) {
        std::memcpy(chunk1.get(), input.data(), len);
    }
    chunk1[len] = '\0';

    // Copy chunk1 -> chunk2 with bounds checking
    if (len > 0) {
        std::memcpy(chunk2.get(), chunk1.get(), len);
    }
    chunk2[len] = '\0';

    // Print second chunk
    std::cout << chunk2.get() << std::endl;

    // "Free" first chunk (zeroize then reset)
    if (len > 0) {
        std::memset(chunk1.get(), 0, len + 1);
    }
    chunk1.reset();

    // Print second chunk again (should remain unchanged)
    std::cout << chunk2.get() << std::endl;

    // Capture return value
    std::string ret(chunk2.get(), len);

    // "Free" second chunk (zeroize then reset)
    if (len > 0) {
        std::memset(chunk2.get(), 0, len + 1);
    }
    chunk2.reset();

    return ret;
}

int main() {
    const std::string tests[5] = {
        "Hello, world!",
        "",
        "A longer string with multiple words.",
        "Unicode: 😊🚀✨",
        "Line1\nLine2\tTabbed"
    };

    for (const auto& t : tests) {
        std::string result = demoCopy(t);
        std::cout << "Returned: " << result << std::endl;
        std::cout << "---" << std::endl;
    }
    return 0;
}