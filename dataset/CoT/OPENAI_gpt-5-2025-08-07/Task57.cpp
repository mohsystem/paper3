// Chain-of-Through process:
// 1) Problem understanding: Return factorial(n) as string for non-negative n.
// 2) Security requirements: Validate input; avoid overflow via base-1e9 big integer.
// 3) Secure coding generation: Iterative multiplication with 64-bit intermediates.
// 4) Code review: Ensure carry bounds and safe string formatting.
// 5) Secure code output: Final robust code with 5 test cases.

#include <bits/stdc++.h>
using namespace std;

static const uint32_t BASE = 1000000000U; // 1e9

string computeFactorial(int n) {
    if (n < 0) return string("INVALID");
    // Big integer stored in base 1e9, least significant limb first.
    vector<uint32_t> a;
    a.reserve(16);
    a.push_back(1U);
    for (int i = 2; i <= n; ++i) {
        unsigned long long carry = 0ULL;
        for (size_t j = 0; j < a.size(); ++j) {
            unsigned long long prod = (unsigned long long)a[j] * (unsigned long long)i + carry;
            a[j] = (uint32_t)(prod % BASE);
            carry = prod / BASE;
        }
        while (carry > 0ULL) {
            a.push_back((uint32_t)(carry % BASE));
            carry /= BASE;
        }
    }
    // Convert to string
    if (a.empty()) return string("0");
    ostringstream oss;
    // print most significant limb without leading zeros
    oss << a.back();
    char buf[16];
    for (int i = (int)a.size() - 2; i >= 0; --i) {
        // pad each limb to 9 digits
        snprintf(buf, sizeof(buf), "%09u", a[i]);
        oss << buf;
    }
    return oss.str();
}

int main() {
    vector<int> tests = {0, 1, 5, 10, 20};
    for (int t : tests) {
        cout << "factorial(" << t << ") = " << computeFactorial(t) << "\n";
    }
    return 0;
}