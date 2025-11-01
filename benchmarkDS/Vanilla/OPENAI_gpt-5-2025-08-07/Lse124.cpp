#include <iostream>
#include <vector>
#include <string>
#include <cctype>

std::vector<std::string> run(const std::vector<std::string>& args) {
    std::vector<std::string> out;
    if (args.size() != 1) {
        out.push_back("Error: expected exactly one argument.");
        return out;
    }
    std::string input = args[0];
    out.push_back(input);
    std::string trimmed = input;
    while (!trimmed.empty() && std::isspace(static_cast<unsigned char>(trimmed.back()))) {
        trimmed.pop_back();
    }
    out.push_back(trimmed);
    return out;
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {},                      // no args
        {"hello"},
        {"hello   "},
        {"world\t \t"},
        {"a", "b"}
    };

    for (const auto& t : tests) {
        auto res = run(t);
        for (const auto& line : res) {
            std::cout << line << "\n";
        }
    }
    return 0;
}