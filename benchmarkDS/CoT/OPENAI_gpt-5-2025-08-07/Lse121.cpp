#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

char* copy_and_manage(const std::string& input) {
    size_t n = input.size() + 1; // include null terminator
    char* first = static_cast<char*>(std::malloc(n));
    if (!first) return nullptr;
    std::memcpy(first, input.c_str(), n);

    char* second = static_cast<char*>(std::malloc(n));
    if (!second) {
        std::free(first);
        return nullptr;
    }
    std::memcpy(second, first, n);

    // Free first chunk
    std::free(first);

    return second; // caller prints and frees
}

int main() {
    std::vector<std::string> tests = {"Hello", "World!", "Memory Test", "12345", "Symbols!@#"};
    for (const auto& s : tests) {
        char* second = copy_and_manage(s);
        if (!second) {
            std::cerr << "Allocation failed\n";
            continue;
        }
        std::cout << second << "\n";
        std::cout << second << "\n";
        // Free second chunk
        std::free(second);
    }
    return 0;
}