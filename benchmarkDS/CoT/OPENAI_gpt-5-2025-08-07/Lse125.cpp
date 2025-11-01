#include <iostream>
#include <string>
#include <cctype>

std::string trim_trailing(const std::string& s) {
    size_t i = s.size();
    while (i > 0 && std::isspace(static_cast<unsigned char>(s[i - 1]))) {
        --i;
    }
    return s.substr(0, i);
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cout << trim_trailing(argv[1]) << std::endl;
    } else {
        const std::string tests[5] = {
            "Hello World   ",
            "TrailingTabs\t\t\t",
            "LineEnd\n\n",
            "  LeadingShouldStay   ",
            "    "
        };
        for (const auto& t : tests) {
            std::cout << trim_trailing(t) << std::endl;
        }
    }
    return 0;
}