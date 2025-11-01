#include <iostream>
#include <string>
#include <vector>

class Lse123 {
public:
    static std::string process(const std::vector<std::string>& args) {
        // First if: ensure exactly one argument
        if (args.size() != 1) {
            return std::string();
        }
        std::string s = args[0];

        // For loop trims trailing whitespace (spaces and tabs)
        if (!s.empty()) {
            std::size_t end = s.size();
            for (; end > 0 && (s[end - 1] == ' ' || s[end - 1] == '\t'); --end) {
                // trimming
            }
            s.resize(end);
        }

        // Second for loop trims the newlines (removes all '\n' and '\r')
        std::string out;
        out.reserve(s.size());
        for (char c : s) {
            if (c != '\n' && c != '\r') {
                out.push_back(c);
            }
        }

        // C++ std::string is internally managed; no manual null-termination necessary
        return out;
    }
};

int main() {
    std::vector<std::vector<std::string>> tests = {
        {"Hello World   "},
        {"Line with newline\n\n"},
        {" Mix \t \nLine\r\nBreaks\t\t"},
        {},                 // invalid: 0 args
        {"A", "B"}          // invalid: >1 args
    };

    for (std::size_t i = 0; i < tests.size(); ++i) {
        std::string res = Lse123::process(tests[i]);
        std::cout << "Test " << (i + 1) << ": \"" << res << "\"\n";
    }
    return 0;
}