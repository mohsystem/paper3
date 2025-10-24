#include <iostream>
#include <vector>
#include <string>

std::string odd_or_even(const std::vector<long long>& arr) {
    unsigned int parity = 0u;
    for (long long x : arr) {
        parity ^= static_cast<unsigned long long>(x) & 1ull;
    }
    return parity ? "odd" : "even";
}

int main() {
    std::vector<std::vector<long long>> tests = {
        {0},
        {0, 1, 4},
        {0, -1, -5},
        {},
        {2, 2, 2, 3}
    };

    for (const auto& t : tests) {
        std::cout << "[";
        for (size_t i = 0; i < t.size(); ++i) {
            std::cout << t[i];
            if (i + 1 < t.size()) std::cout << ", ";
        }
        std::cout << "] -> " << odd_or_even(t) << "\n";
    }
    return 0;
}