#include <iostream>
#include <string>
#include <vector>

int add1000(int x) {
    return x + 1000;
}

int main(int argc, char* argv[]) {
    if (argc >= 2) {
        try {
            int x = std::stoi(std::string(argv[1]));
            int result = add1000(x);
            std::cout << result << std::endl;
            return 0;
        } catch (...) {
            // Fall through to run test cases on invalid input
        }
    }
    std::vector<int> tests = { -1000, -1, 0, 1, 12345 };
    for (int t : tests) {
        std::cout << add1000(t) << std::endl;
    }
    return 0;
}