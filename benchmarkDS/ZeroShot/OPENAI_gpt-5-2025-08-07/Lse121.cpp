#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <limits>

std::string process(const std::string& input) {
    const std::string& safeInput = input;
    size_t len = safeInput.size();

    if (len > (std::numeric_limits<size_t>::max)() - 1) {
        return std::string();
    }

    char* chunk1 = static_cast<char*>(std::malloc(len + 1));
    if (!chunk1) {
        return std::string();
    }
    std::memcpy(chunk1, safeInput.data(), len);
    chunk1[len] = '\0';

    char* chunk2 = static_cast<char*>(std::malloc(len + 1));
    if (!chunk2) {
        std::free(chunk1);
        return std::string();
    }
    std::memcpy(chunk2, chunk1, len);
    chunk2[len] = '\0';

    std::cout << chunk2 << std::endl;

    std::free(chunk1);
    chunk1 = nullptr;

    std::cout << chunk2 << std::endl;

    std::string result(chunk2);
    result.push_back('\n');
    result += chunk2;
    result.push_back('\n');

    std::free(chunk2);
    chunk2 = nullptr;

    return result;
}

int main() {
    std::string tests[5] = {
        "Hello, World!",
        "",
        "UTF-8: こんにちは",
        "Numbers: 1234567890",
        "Line1\nLine2\tTabbed!"
    };

    for (const auto& t : tests) {
        std::string out = process(t);
        std::cout << "Returned:" << std::endl;
        std::cout << out;
    }
    return 0;
}