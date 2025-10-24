#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <vector>

std::vector<std::string> matchLinesStartNumEndDot(const std::string& filePath) {
    std::vector<std::string> matches;
    std::ifstream in(filePath, std::ios::in);
    if (!in.is_open()) {
        return matches;
    }
    std::regex pattern(R"(^\d+.*\.$)");
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (std::regex_match(line, pattern)) {
            matches.push_back(line);
        }
    }
    return matches;
}

static bool writeFile(const std::string& path, const std::vector<std::string>& lines) {
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;
    for (const auto& s : lines) {
        out << s << "\n";
    }
    return true;
}

static void printResult(const std::string& title, const std::vector<std::string>& v) {
    std::cout << title << "\n";
    for (const auto& s : v) {
        std::cout << s << "\n";
    }
    std::cout << "----\n";
}

int main() {
    // Test case 1
    std::string f1 = "task99_cpp_test1.txt";
    writeFile(f1, {
        "123 This line starts with numbers and ends with a dot.",
        "Not starting with number.",
        "42.",
        "7 ends without dot",
        "007 Bond."
    });
    printResult("Test 1:", matchLinesStartNumEndDot(f1));

    // Test case 2
    std::string f2 = "task99_cpp_test2.txt";
    writeFile(f2, {
        "1.",
        "22.",
        "333.",
        "4444."
    });
    printResult("Test 2:", matchLinesStartNumEndDot(f2));

    // Test case 3
    std::string f3 = "task99_cpp_test3.txt";
    writeFile(f3, {
        "hello.",
        ".",
        " no number.",
        "123 but no dot at end",
        ""
    });
    printResult("Test 3:", matchLinesStartNumEndDot(f3));

    // Test case 4
    std::string f4 = "task99_cpp_test4.txt";
    writeFile(f4, {
        "0.",
        "9 Ends without dot",
        "123abc.",
        "999!.",
        "001 leading zeros."
    });
    printResult("Test 4:", matchLinesStartNumEndDot(f4));

    // Test case 5
    std::string f5 = "task99_cpp_test5.txt";
    writeFile(f5, {
        "123.456.",
        "10. end.",
        "5... ... ...",
        "8.",
        "12.3"
    });
    printResult("Test 5:", matchLinesStartNumEndDot(f5));

    return 0;
}