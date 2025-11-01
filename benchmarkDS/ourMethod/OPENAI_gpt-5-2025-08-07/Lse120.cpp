#include <iostream>
#include <vector>
#include <string>
#include <optional>
#include <cstring>

static constexpr std::size_t MAX_SIZE = 1u << 20; // 1 MiB

// Allocates two chunks, copies the first into the second, returns the second as a string.
// Returns std::nullopt if input is too large.
std::optional<std::string> copyChunk(const std::string& input) {
    const std::size_t len = input.size();
    if (len > MAX_SIZE) {
        return std::nullopt;
    }

    // Allocate first chunk and copy input into it
    std::vector<char> first(len);
    if (len > 0) {
        std::memcpy(first.data(), input.data(), len);
    }

    // Allocate second chunk and copy first into it
    std::vector<char> second(len);
    if (len > 0) {
        std::memcpy(second.data(), first.data(), len);
    }

    return std::string(second.data(), second.size());
}

static void runTest(const std::string& s, const std::string& label) {
    std::cout << "Test: " << label << "\n";
    auto out = copyChunk(s);
    if (!out.has_value()) {
        std::cout << "[ERROR] Copy failed or input too large.\n";
    } else {
        std::cout << out.value() << "\n";
    }
    std::cout << "----\n";
}

int main() {
    // Ensure the source file is saved as UTF-8 for proper unicode literal handling.

    // Test 1: Empty
    runTest(std::string(), "Empty");

    // Test 2: Simple text
    runTest(std::string("Hello, world!"), "Simple text");

    // Test 3: Unicode text (UTF-8)
    runTest(std::string("Unicode test: 你好, мир, مرحبا"), "Unicode text");

    // Test 4: Large but within limit
    runTest(std::string(100000, 'A'), "Large within limit");

    // Test 5: Oversized (exceeds MAX_SIZE)
    runTest(std::string(MAX_SIZE + 1, 'B'), "Oversized");

    return 0;
}