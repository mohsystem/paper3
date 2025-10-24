#include <iostream>
#include <string>
#include <vector>
#include <cctype>

std::string toJadenCase(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    bool newWord = true;
    for (unsigned char uch : s) {
        char ch = static_cast<char>(uch);
        if (newWord && std::isalpha(uch)) {
            out.push_back(static_cast<char>(std::toupper(uch)));
        } else if (std::isalpha(uch)) {
            out.push_back(static_cast<char>(std::tolower(uch)));
        } else {
            out.push_back(ch);
        }
        newWord = std::isspace(uch) || ch == '-';
    }
    return out;
}

int main() {
    std::vector<std::string> tests = {
        "How can mirrors be real if our eyes aren't real",
        "",
        "not jaden-cased string",
        "this IS a tESt",
        "   hello\tworld\nit's me - isn't it?"
    };
    for (const auto& t : tests) {
        std::cout << "Input:  " << t << "\n";
        std::cout << "Output: " << toJadenCase(t) << "\n";
        std::cout << "---\n";
    }
    return 0;
}