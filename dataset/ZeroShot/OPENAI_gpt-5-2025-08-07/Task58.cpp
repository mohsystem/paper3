#include <iostream>
#include <string>

bool XO(const std::string& s) {
    int x = 0, o = 0;
    for (char c : s) {
        if (c == 'x' || c == 'X') ++x;
        else if (c == 'o' || c == 'O') ++o;
    }
    return x == o;
}

int main() {
    std::string tests[] = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
    std::cout << std::boolalpha;
    for (const auto& t : tests) {
        std::cout << XO(t) << "\n";
    }
    return 0;
}