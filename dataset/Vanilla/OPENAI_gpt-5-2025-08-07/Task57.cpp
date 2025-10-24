#include <iostream>
#include <vector>

unsigned long long factorial(unsigned int n) {
    unsigned long long result = 1ULL;
    for (unsigned int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    std::vector<unsigned int> tests = {0, 1, 5, 10, 20};
    for (auto t : tests) {
        std::cout << t << "! = " << factorial(t) << std::endl;
    }
    return 0;
}