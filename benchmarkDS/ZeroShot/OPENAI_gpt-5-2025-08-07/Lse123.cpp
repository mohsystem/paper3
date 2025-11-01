#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

static bool isWhitespaceButNotNewline(char c) {
    return c == ' ' || c == '\t' || c == '\v' || c == '\f';
}

static bool isNewline(char c) {
    return c == '\n' || c == '\r';
}

std::string sanitize_argument_cpp(const std::string& s) {
    long long end = static_cast<long long>(s.size()) - 1;

    for (; end >= 0 && isWhitespaceButNotNewline(s[static_cast<size_t>(end)]); --end) {
        // trimming trailing whitespace except newlines
    }
    for (; end >= 0 && isNewline(s[static_cast<size_t>(end)]); --end) {
        // trimming trailing newlines
    }

    size_t newLen = (end >= 0) ? static_cast<size_t>(end) + 1 : 0u;
    return s.substr(0, newLen);
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        try {
            std::string input(argv[1] ? argv[1] : "");
            std::string output = sanitize_argument_cpp(input);
            std::cout << output << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            return 1;
        }
    } else {
        // 5 test cases
        std::vector<std::string> tests = {
            "Hello World   ",
            "Tabs and spaces\t \t   ",
            "Ends with newlines\n\n\r",
            "Mix \t\v\f\n\r",
            ""
        };
        for (const auto& t : tests) {
            std::string res = sanitize_argument_cpp(t);
            std::cout << "[" << res << "]" << std::endl;
        }
    }
    return 0;
}