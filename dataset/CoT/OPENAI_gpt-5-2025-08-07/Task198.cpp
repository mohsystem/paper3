#include <bits/stdc++.h>
using namespace std;

// Chain-of-Through Steps 1-5: Implement rand10 via rand7 using secure rejection sampling,
// and an internal PRNG (no built-in randomness).

class Task198 {
private:
    static uint64_t rngState; // non-zero seed

    static inline uint64_t next64() {
        uint64_t x = rngState;
        x ^= (x << 13);
        x ^= (x >> 7);
        x ^= (x << 17);
        rngState = x;
        return x & 0x7fffffffffffffffULL; // 63-bit non-negative
    }

public:
    static int rand7() {
        const uint32_t limit = 2147483646u; // 7 * floor(2^31/7)
        for (;;) {
            uint32_t v = static_cast<uint32_t>(next64() & 0x7fffffffULL);
            if (v <= limit) return static_cast<int>(v % 7u) + 1;
        }
    }

    static int rand10() {
        for (;;) {
            int a = rand7();
            int b = rand7();
            int idx = (a - 1) * 7 + b; // 1..49
            if (idx <= 40) return 1 + (idx - 1) % 10;
        }
    }

    static vector<int> runTest(int n) {
        if (n < 0) throw invalid_argument("n must be non-negative");
        vector<int> out;
        out.reserve(n);
        for (int i = 0; i < n; ++i) out.push_back(rand10());
        return out;
    }

    static void printArray(const vector<int>& arr) {
        cout << '[';
        for (size_t i = 0; i < arr.size(); ++i) {
            if (i) cout << ',';
            cout << arr[i];
        }
        cout << ']' << '\n';
    }
};

uint64_t Task198::rngState = 0x123456789ABCDEFULL;

int main() {
    vector<int> tests = {1, 2, 3, 5, 20}; // 5 test cases
    for (int n : tests) {
        auto res = Task198::runTest(n);
        Task198::printArray(res);
    }
    return 0;
}