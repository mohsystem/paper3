#include <iostream>
#include <string>
#include <cmath>
#include <vector>

int digPow(int n, int p) {
    if (n <= 0 || p <= 0) {
        return -1; // As per problem statement, n and p are strictly positive
    }
    
    std::string s = std::to_string(n);
    long long sum = 0;
    
    for (char c : s) {
        int digit = c - '0';
        long long term = static_cast<long long>(round(pow(digit, p)));
        // A simple overflow check
        if (sum > __LONG_LONG_MAX__ - term) {
            // Handle potential overflow if sum becomes very large
            // For this problem's constraints, it might not be necessary,
            // but it's good practice. Returning -1 could indicate an error/overflow.
            return -1;
        }
        sum += term;
        p++;
    }

    if (sum % n == 0) {
        return static_cast<int>(sum / n);
    } else {
        return -1;
    }
}

int main() {
    struct TestCase {
        int n;
        int p;
        int expected;
    };

    std::vector<TestCase> testCases = {
        {89, 1, 1},
        {92, 1, -1},
        {695, 2, 2},
        {46288, 3, 51},
        {135, 1, 1}
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        int result = digPow(testCases[i].n, testCases[i].p);
        std::cout << "Test Case " << i + 1 << ": n=" << testCases[i].n << ", p=" << testCases[i].p
                  << " -> Result: " << result << " (Expected: " << testCases[i].expected << ")" << std::endl;
    }

    return 0;
}