#include <iostream>
#include <string>
#include <vector>
#include <cctype>

std::string trim_trailing_whitespace(const std::string& s) {
    std::string::size_type end = s.size();
    while (end > 0 && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(0, end);
}

int main(int argc, char* argv[]) {
    // Command-line behavior
    if (argc > 1) {
        std::string input(argv[1]);
        std::string output = trim_trailing_whitespace(input);
        std::cout << output << std::endl;
    }

    // 5 test cases
    std::vector<std::string> tests = {
        "hello   ",
        " no-trim",
        "tabs\t\t",
        "mixed spaces\t ",
        ""
    };
    for (const auto& t : tests) {
        std::cout << trim_trailing_whitespace(t) << std::endl;
    }
    return 0;
}