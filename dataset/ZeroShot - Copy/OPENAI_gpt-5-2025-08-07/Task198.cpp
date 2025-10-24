#include <bits/stdc++.h>
using namespace std;

class RNG {
    uint32_t state;
public:
    explicit RNG(uint64_t seed) {
        // SplitMix64 to mix seed, then take lower 32 bits; ensure non-zero
        uint64_t z = seed + 0x9E3779B97F4A7C15ULL;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        z ^= (z >> 31);
        state = static_cast<uint32_t>(z);
        if (state == 0) state = 0x6D2B79F5u;
    }

    uint32_t nextUInt32() {
        uint32_t x = state;
        x ^= (x << 13);
        x ^= (x >> 17);
        x ^= (x << 5);
        state = x;
        return x;
    }

    int rand7() {
        const uint32_t LIMIT = 4294967291u; // floor((2^32)/7)*7 - 1
        while (true) {
            uint32_t u = nextUInt32();
            if (u <= LIMIT) {
                return 1 + static_cast<int>(u % 7u);
            }
        }
    }

    int rand10() {
        while (true) {
            int a = rand7();
            int b = rand7();
            int idx = (a - 1) * 7 + b; // 1..49
            if (idx <= 40) {
                return 1 + (idx - 1) % 10;
            }
        }
    }
};

vector<int> runTest(int n, uint64_t seed) {
    if (n < 0) throw invalid_argument("n must be non-negative");
    RNG rng(seed);
    vector<int> res;
    res.reserve(n);
    for (int i = 0; i < n; ++i) res.push_back(rng.rand10());
    return res;
}

int main() {
    vector<int> tests = {1, 2, 3, 5, 10};
    vector<uint64_t> seeds = {12345ULL, 67890ULL, 13579ULL, 24680ULL, 424242ULL};
    for (size_t i = 0; i < tests.size(); ++i) {
        auto out = runTest(tests[i], seeds[i]);
        cout << "[";
        for (size_t j = 0; j < out.size(); ++j) {
            if (j) cout << ",";
            cout << out[j];
        }
        cout << "]\n";
    }
    return 0;
}