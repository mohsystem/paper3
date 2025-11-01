#include <iostream>
#include <string>
#include <cctype>

std::string trimTrailing(const std::string& s) {
    std::string out = s;
    while (!out.empty() && std::isspace(static_cast<unsigned char>(out.back()))) {
        out.pop_back();
    }
    return out;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::cout << trimTrailing(argv[1]) << std::endl;
    } else {
        std::string tests[5] = {
            "hello   ",
            "test\t\t",
            "line\n",
            " mix  \t\n",
            "no-trim"
        };
        for (const auto& t : tests) {
            std::cout << trimTrailing(t) << std::endl;
        }
    }
    return 0;
}