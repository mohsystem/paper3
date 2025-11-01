#include <iostream>
#include <string>
#include <vector>
#include <cctype>

static const size_t MAX_LEN = 1048576; // 1 MiB

static std::string rtrim_copy(const std::string& s) {
    size_t end = s.size();
    while (end > 0 && std::isspace(static_cast<unsigned char>(s[end - 1])) != 0) {
        --end;
    }
    return s.substr(0, end);
}

int process(const std::vector<std::string>& argv) {
    if (argv.size() != 1) {
        std::cerr << "Error: expected exactly one argument." << std::endl;
        return 1;
    }

    const std::string& input = argv[0];
    if (input.size() > MAX_LEN) {
        std::cerr << "Error: input too long." << std::endl;
        return 1;
    }

    std::cout << input << std::endl;
    std::string trimmed = rtrim_copy(input);
    std::cout << trimmed << std::endl;

    return 0;
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {"hello"},
        {"hello \t \n"},
        {},
        {"a", "b"},
        {"   "}
    };

    int caseNo = 1;
    for (const auto& t : tests) {
        std::cout << "---- Test " << caseNo << " ----" << std::endl;
        int code = process(t);
        std::cout << "Exit code: " << code << std::endl;
        ++caseNo;
    }
    return 0;
}