#include <iostream>
#include <string>
#include <cctype>

bool XO(const std::string& str) {
    int xCount = 0;
    int oCount = 0;
    for (char c : str) {
        char lower_c = std::tolower(static_cast<unsigned char>(c));
        if (lower_c == 'x') {
            xCount++;
        } else if (lower_c == 'o') {
            oCount++;
        }
    }
    return xCount == oCount;
}

int main() {
    std::string testCases[] = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
    for (const std::string& test : testCases) {
        std::cout << "XO(\"" << test << "\") => " << std::boolalpha << XO(test) << std::endl;
    }
    return 0;
}