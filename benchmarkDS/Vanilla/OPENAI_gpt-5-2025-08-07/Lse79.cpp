#include <iostream>
#include <string>
#include <cstring>
#include <algorithm>
#include <vector>

std::string alloc_and_copy_100(const std::string& input) {
    char* buf = (char*)std::malloc(1024);
    if (!buf) return std::string();
    size_t len = std::min<size_t>(100, std::min<size_t>(input.size(), 1023));
    if (len > 0) std::memcpy(buf, input.data(), len);
    buf[len] = '\0';
    std::string result(buf, len);
    std::free(buf);
    return result;
}

std::string alloc_and_copy_100_second(const std::string& input) {
    char* buf1 = (char*)std::malloc(1024);
    if (!buf1) return std::string();
    size_t len = std::min<size_t>(100, std::min<size_t>(input.size(), 1023));
    if (len > 0) std::memcpy(buf1, input.data(), len);
    buf1[len] = '\0';
    std::string result(buf1, len);
    std::free(buf1);
    return result;
}

std::string read_up_to_1024(const std::string& input) {
    char* buf = (char*)std::malloc(1024);
    if (!buf) return std::string();
    size_t len = std::min<size_t>(input.size(), 1024);
    if (len > 0) std::memcpy(buf, input.data(), len);
    // Ensure null-termination for safe string construction
    size_t safeLen = std::min<size_t>(len, 1023);
    buf[safeLen] = '\0';
    std::string result(buf, len);
    std::free(buf);
    return result;
}

int main() {
    std::vector<std::string> tests;
    tests.push_back("Hello World");
    tests.push_back(std::string(100, 'A'));
    tests.push_back(std::string(150, 'B'));
    tests.push_back("");
    tests.push_back(std::string("Unicode 😃🚀 ") + std::string(1100, 'C'));

    for (size_t i = 0; i < tests.size(); ++i) {
        std::string r1 = alloc_and_copy_100(tests[i]);
        std::string r2 = alloc_and_copy_100_second(tests[i]);
        std::string r3 = read_up_to_1024(tests[i]);
        std::cout << "Test " << (i + 1) << "\n";
        std::cout << "alloc_and_copy_100: " << r1 << "\n";
        std::cout << "alloc_and_copy_100_second: " << r2 << "\n";
        std::cout << "read_up_to_1024: " << (r3.size() > 80 ? r3.substr(0, 80) + "...(truncated print)" : r3) << "\n";
        std::cout << "---\n";
    }

    return 0;
}