#include <iostream>
#include <string>
#include <vector>
#include <cctype>

std::string trim_trailing_whitespace(const std::string& s) {
    std::size_t end = s.size();
    while (end > 0 && std::isspace(static_cast<unsigned char>(s[end - 1])) != 0) {
        --end;
    }
    return s.substr(0, end);
}

int main(int argc, char* argv[]) {
    // If a command-line argument is provided, trim trailing whitespace and print it.
    if (argc > 1 && argv[1] != nullptr) {
        std::string arg(argv[1]);
        std::cout << trim_trailing_whitespace(arg) << std::endl;
    }

    // Five test cases
    std::vector<std::string> tests{
        "hello",
        "hello   ",
        "line\n",
        "tabs\t\t",
        "mix \t\n\r"
    };

    for (std::size_t i = 0; i < tests.size(); ++i) {
        std::string res = trim_trailing_whitespace(tests[i]);
        std::cout << "TEST " << (i + 1) << ": [" << res << "]" << std::endl;
    }

    return 0;
}