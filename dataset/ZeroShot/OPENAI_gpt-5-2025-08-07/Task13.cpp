#include <iostream>
#include <vector>
#include <string>

bool isValidWalk(const std::vector<std::string>& walk) {
    if (walk.size() != 10) return false;
    int x = 0, y = 0;
    for (const auto& dir : walk) {
        if (dir.size() != 1) return false;
        char c = dir[0];
        switch (c) {
            case 'n': ++y; break;
            case 's': --y; break;
            case 'e': ++x; break;
            case 'w': --x; break;
            default: return false;
        }
    }
    return x == 0 && y == 0;
}

int main() {
    std::vector<std::vector<std::string>> tests = {
        {"n","s","n","s","n","s","n","s","n","s"},
        {"w","e","w","e","w","e","w","e","w","e","w","e"},
        {"w","w","w","w","w","w","w","w","w","w"},
        {"n","s","n","s","n","s","n","s","n","x"},
        {"n","n","e","e","s","s","w","w","n","s"}
    };
    bool expected[] = {true, false, false, false, true};
    for (size_t i = 0; i < tests.size(); ++i) {
        bool result = isValidWalk(tests[i]);
        std::cout << "Test " << (i+1) << ": " << (result ? "true" : "false")
                  << " (expected " << (expected[i] ? "true" : "false") << ")\n";
    }
    return 0;
}