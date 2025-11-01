#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

std::string copyAndPrint(const std::string& input) {
    std::vector<char> src(input.begin(), input.end());
    std::vector<char> dst(src.size());
    if (!src.empty()) {
        std::copy(src.begin(), src.end(), dst.begin());
    }
    std::string output(dst.begin(), dst.end());
    std::cout << output << std::endl;
    return output;
}

int main() {
    std::string tests[5] = {
        "",
        "Hello, World!",
        "Secure Copy 123!@#",
        "こんにちは",
        std::string(100, 'a')
    };

    for (const auto& t : tests) {
        copyAndPrint(t);
    }
    return 0;
}