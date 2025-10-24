#include <iostream>
#include <string>
#include <cctype>

bool XO(const std::string& s) {
    int x = 0, o = 0;
    for (char ch : s) {
        char c = std::tolower(static_cast<unsigned char>(ch));
        if (c == 'x') ++x;
        else if (c == 'o') ++o;
    }
    return x == o;
}

int main() {
    std::string tests[] = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
    std::cout << std::boolalpha;
    for (const auto& t : tests) {
        std::cout << "XO(\"" << t << "\") => " << XO(t) << "\n";
    }
    return 0;
}