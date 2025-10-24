#include <iostream>

int persistence(unsigned long long num) {
    int steps = 0;
    while (num >= 10ULL) {
        unsigned long long prod = 1ULL;
        unsigned long long t = num;
        while (t > 0ULL) {
            prod *= (t % 10ULL);
            t /= 10ULL;
        }
        num = prod;
        steps++;
    }
    return steps;
}

int main() {
    unsigned long long tests[] = {39ULL, 999ULL, 4ULL, 25ULL, 77ULL};
    for (unsigned long long n : tests) {
        std::cout << "persistence(" << n << ") = " << persistence(n) << std::endl;
    }
    return 0;
}