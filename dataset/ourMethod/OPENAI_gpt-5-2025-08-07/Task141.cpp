#include <iostream>
#include <string>
#include <stdexcept>

static const std::size_t MAX_LEN = 1000000;

std::string reverseString(const std::string& s) {
    if (s.size() > MAX_LEN) {
        throw std::invalid_argument("Input too long");
    }
    std::string r;
    r.reserve(s.size());
    for (auto it = s.rbegin(); it != s.rend(); ++it) {
        r.push_back(*it);
    }
    return r;
}

int main() {
    const std::string tests[5] = {
        "",
        "a",
        "abc",
        "Hello, World!",
        "12345!@#$%"
    };
    for (const auto& t : tests) {
        std::cout << reverseString(t) << '\n';
    }
    return 0;
}