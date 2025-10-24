// Step 1: Problem understanding and definition
// Step 2: Security requirements consideration
// Step 3: Secure coding generation
// Step 4: Code review simulated through comments
// Step 5: Secure code output

#include <iostream>
#include <vector>
#include <string>

std::string oddOrEven(const std::vector<long long>& arr) {
    int parity = 0; // 0 for even, 1 for odd
    for (long long n : arr) {
        parity ^= static_cast<int>(n & 1LL); // safe for negatives
    }
    return parity == 0 ? "even" : "odd";
}

int main() {
    std::vector<std::vector<long long>> tests;
    tests.push_back(std::vector<long long>{});           // even
    tests.push_back({0});                                // even
    tests.push_back({0, 1, 4});                          // odd
    tests.push_back({0, -1, -5});                        // even
    tests.push_back({2, 2, 2, 2});                       // even

    for (const auto& t : tests) {
        std::cout << oddOrEven(t) << "\n";
    }
    return 0;
}