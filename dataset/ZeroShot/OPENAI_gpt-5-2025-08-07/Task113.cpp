#include <iostream>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

std::string concatenate(const std::vector<std::string>& parts) {
    size_t total = 0;
    for (const auto& s : parts) {
        if (s.size() > std::numeric_limits<size_t>::max() - total - 1) {
            throw std::length_error("Input too large to concatenate safely.");
        }
        total += s.size();
    }
    std::string result;
    result.reserve(total);
    for (const auto& s : parts) {
        result += s;
    }
    return result;
}

int main() {
    try {
        // Test case 1
        std::vector<std::string> t1 = {"Hello", " ", "World", "!"};
        std::cout << "Test 1: " << concatenate(t1) << "\n";

        // Test case 2 (Unicode - UTF-8 literals)
        std::vector<std::string> t2 = {"na\xC3\xAFve", " ", "caf\xC3\xA9", " ", "\xF0\x9F\x98\x8A"};
        std::cout << "Test 2: " << concatenate(t2) << "\n";

        // Test case 3 (empty entries)
        std::vector<std::string> t3 = {"A", "", "B", "", "C"};
        std::cout << "Test 3: " << concatenate(t3) << "\n";

        // Test case 4 (empty vector)
        std::vector<std::string> t4 = {};
        std::cout << "Test 4: " << concatenate(t4) << "\n";

        // Test case 5 (mix of empty and non-empty)
        std::vector<std::string> t5 = {"", "", "x", "", "y", "", "z"};
        std::cout << "Test 5: " << concatenate(t5) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
    return 0;
}