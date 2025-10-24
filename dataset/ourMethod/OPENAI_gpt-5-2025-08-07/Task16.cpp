#include <iostream>
#include <string>

bool solution(const std::string& str, const std::string& ending) {
    // Validate inputs: treat null-like cases via empty strings (handled by std::string)
    if (ending.size() > str.size()) {
        return false;
    }
    return str.compare(str.size() - ending.size(), ending.size(), ending) == 0;
}

int main() {
    struct TestCase {
        std::string s;
        std::string e;
        bool expected;
    } tests[5] = {
        {"abc", "bc", true},
        {"abc", "d", false},
        {"hello", "", true},
        {"testing", "ing", true},
        {"short", "longer", false}
    };

    for (const auto& t : tests) {
        bool result = solution(t.s, t.e);
        std::cout << "solution(\"" << t.s << "\", \"" << t.e << "\") = "
                  << (result ? "true" : "false")
                  << " | expected: " << (t.expected ? "true" : "false") << std::endl;
    }
    return 0;
}