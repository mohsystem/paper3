#include <iostream>
#include <string>
#include <vector>
#include <cctype>

struct Result {
    std::vector<std::string> lines;
    int exit_code;
};

static std::string rtrim(const std::string& s) {
    if (s.empty()) return s;
    std::size_t end = s.size();
    while (end > 0 && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(0, end);
}

// Function that accepts input as parameters and returns the output (and exit code)
Result process_args(const std::vector<std::string>& args) {
    Result res;
    if (args.size() != 1) {
        res.lines.push_back("Error: expected exactly one argument.");
        res.exit_code = 1;
        return res;
    }
    const std::string& input = args[0];
    res.lines.push_back(input);
    std::string trimmed = rtrim(input);
    res.lines.push_back(trimmed);
    res.exit_code = 0;
    return res;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::vector<std::string> real_args;
        for (int i = 1; i < argc; ++i) {
            real_args.emplace_back(argv[i] ? argv[i] : "");
        }
        Result r = process_args(real_args);
        for (const auto& line : r.lines) {
            std::cout << line << "\n";
        }
        return r.exit_code;
    } else {
        // 5 test cases
        std::vector<std::vector<std::string>> tests = {
            {},                 // 1) zero args -> error
            {"hello"},          // 2) single arg
            {"abc   "},         // 3) trailing spaces
            {"foo \t"},         // 4) trailing tab
            {"one", "two"}      // 5) multiple args -> error
        };
        for (std::size_t i = 0; i < tests.size(); ++i) {
            std::cout << "=== Test " << (i + 1) << " ===\n";
            Result r = process_args(tests[i]);
            for (const auto& line : r.lines) {
                std::cout << line << "\n";
            }
            std::cout << "Exit code: " << r.exit_code << "\n";
        }
    }
    return 0;
}