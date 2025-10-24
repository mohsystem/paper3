#include <iostream>
#include <string>
#include <climits>

struct ProductResult {
    bool ok;
    unsigned long long value;
};

static const size_t MAX_LEN = 1000000;
static const int MAX_SAFE_SPAN = 20; // ensures 9^span fits in unsigned long long

ProductResult largestProduct(const std::string& digits, int span) {
    ProductResult err{false, 0};

    if (digits.size() > MAX_LEN) return err;
    if (span < 0 || static_cast<size_t>(span) > digits.size()) return err;
    for (char c : digits) {
        if (c < '0' || c > '9') return err;
    }
    if (span == 0) return ProductResult{true, 1ULL};
    if (span > MAX_SAFE_SPAN) return err;

    if (span == 1) {
        unsigned long long maxDigit = 0ULL;
        for (char c : digits) {
            unsigned long long d = static_cast<unsigned long long>(c - '0');
            if (d > maxDigit) maxDigit = d;
        }
        return ProductResult{true, maxDigit};
    }

    unsigned long long maxProd = 0ULL;
    const size_t n = digits.size();
    for (size_t i = 0; i + static_cast<size_t>(span) <= n; ++i) {
        unsigned long long prod = 1ULL;
        bool zeroFound = false;
        for (int j = 0; j < span; ++j) {
            unsigned int d = static_cast<unsigned int>(digits[i + j] - '0');
            if (d == 0U) {
                prod = 0ULL;
                zeroFound = true;
                break;
            }
            // Given span <= 20 and digits <= 9, product fits in unsigned long long
            prod *= static_cast<unsigned long long>(d);
        }
        (void)zeroFound;
        if (prod > maxProd) {
            maxProd = prod;
        }
    }
    return ProductResult{true, maxProd};
}

static void runTest(const std::string& digits, int span) {
    ProductResult r = largestProduct(digits, span);
    if (r.ok) {
        std::cout << r.value << "\n";
    } else {
        std::cout << "ERROR\n";
    }
}

int main() {
    // Test 1: Example
    runTest("63915", 3);

    // Test 2: Contains zeros
    runTest("10203", 2);

    // Test 3: Long sequence with span 6
    std::string longSeq = "73167176531330624919225119674426574742355349194934";
    runTest(longSeq, 6);

    // Test 4: Span 0
    runTest("12345", 0);

    // Test 5: 25 nines, span 21 (should error due to overflow guard)
    std::string manyNines(25, '9');
    runTest(manyNines, 21);

    return 0;
}