#include <iostream>
#include <string>
#include <cctype>

std::string toJadenCase(const std::string& s) {
    if (s.empty()) return std::string();
    std::string out;
    out.reserve(s.size());
    bool newWord = true;
    for (unsigned char uch : s) {
        if (std::isspace(uch)) {
            out.push_back(static_cast<char>(uch));
            newWord = true;
        } else {
            if (newWord && std::isalpha(uch)) {
                out.push_back(static_cast<char>(std::toupper(uch)));
            } else if (std::isalpha(uch)) {
                out.push_back(static_cast<char>(std::tolower(uch)));
            } else {
                out.push_back(static_cast<char>(uch));
            }
            newWord = false;
        }
    }
    return out;
}

int main() {
    std::string tests[] = {
        "How can mirrors be real if our eyes aren't real",
        "",
        "most trees are blue",
        "HELLO WORLD",
        "multiple   spaces\tand tabs"
    };
    for (const auto& t : tests) {
        std::cout << toJadenCase(t) << std::endl;
    }
    return 0;
}