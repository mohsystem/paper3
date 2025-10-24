#include <iostream>
#include <string>
#include <vector>

class Task141 {
public:
    static std::string reverse_string(const std::string& s) {
        // Reverse bytes; for UTF-8 this may reverse within multi-byte sequences,
        // but satisfies generic reverse requirement.
        return std::string(s.rbegin(), s.rend());
    }
};

int main() {
    std::vector<std::string> tests = {
        "hello",
        "",
        "A",
        "racecar",
        "Hello, 世界"
    };
    for (const auto& t : tests) {
        std::string r = Task141::reverse_string(t);
        std::cout << "Input: \"" << t << "\" -> Reversed: \"" << r << "\"\n";
    }
    return 0;
}