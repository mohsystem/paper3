#include <iostream>
#include <string>

class Task58 {
public:
    static bool XO(const std::string& s) {
        int x = 0, o = 0;
        for (char c : s) {
            if (c == 'x' || c == 'X') {
                ++x;
            } else if (c == 'o' || c == 'O') {
                ++o;
            }
        }
        return x == o;
    }
};

int main() {
    std::string tests[5] = {"ooxx", "xooxx", "ooxXm", "zpzpzpp", "zzoo"};
    for (const auto& t : tests) {
        std::cout << std::boolalpha << Task58::XO(t) << std::endl;
    }
    return 0;
}