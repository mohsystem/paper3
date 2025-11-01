#include <iostream>
#include <string>
#include <vector>

long long add1000(long long x) {
    return x + 1000;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        try {
            long long val = std::stoll(std::string(argv[1]));
            std::cout << add1000(val) << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Invalid number: " << argv[1] << std::endl;
        }
    }
    std::vector<long long> tests = {
        -1000LL,
        0LL,
        42LL,
        999999999999LL,
        9223372036854774807LL
    };
    for (auto t : tests) {
        std::cout << add1000(t) << std::endl;
    }
    return 0;
}