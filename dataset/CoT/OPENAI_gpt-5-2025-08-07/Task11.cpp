// Chain-of-Through steps applied: understanding, secure design, implementation, review, and safe output.
// C++ implementation with function and 5 test cases.
#include <iostream>
#include <limits>

int nb_year(long long p0, double percent, long long aug, long long p) {
    if (p0 >= p) return 0;
    long double rate = percent / 100.0L;
    long long pop = p0;
    int years = 0;
    while (pop < p) {
        long long prev = pop;
        long long delta = static_cast<long long>(pop * rate); // truncation floors for non-negative rate
        // Check for potential overflow before adding
        if ((aug > 0 && pop > (std::numeric_limits<long long>::max() - aug - delta)) ||
            (aug < 0 && pop < (std::numeric_limits<long long>::min() - aug - delta))) {
            return -1; // overflow risk implies unsafe/unreachable for practical purposes
        }
        pop += delta + aug;
        years++;
        if (pop <= prev || years > 10000000) {
            return -1; // unreachable or guard triggered
        }
    }
    return years;
}

int main() {
    // 5 test cases
    std::cout << nb_year(1000, 2, 50, 1200) << "\n";               // 3
    std::cout << nb_year(1500, 5, 100, 5000) << "\n";              // 15
    std::cout << nb_year(1500000, 2.5, 10000, 2000000) << "\n";    // 10
    std::cout << nb_year(1500000, 0, 10000, 2000000) << "\n";      // 50
    std::cout << nb_year(1000, 0, -1, 1200) << "\n";               // -1
    return 0;
}