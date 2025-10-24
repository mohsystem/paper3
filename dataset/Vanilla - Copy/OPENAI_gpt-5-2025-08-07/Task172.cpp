#include <bits/stdc++.h>
using namespace std;

static const uint64_t MOD1 = 1000000007ULL;
static const uint64_t MOD2 = 1000000009ULL;
static const uint64_t BASE1 = 911382323ULL;
static const uint64_t BASE2 = 972663749ULL;

static inline uint32_t getHash(const vector<uint64_t>& pref, const vector<uint64_t>& pow, uint64_t mod, int l, int r) {
    uint64_t res = (pref[r + 1] + mod - (pref[l] * pow[r - l + 1]) % mod);
    if (res >= mod) res -= mod;
    return (uint32_t)res;
}

int countSquareDistinctSubstrings(const string& s) {
    int n = (int)s.size();
    vector<uint64_t> pow1(n + 1), pow2(n + 1), pref1(n + 1), pref2(n + 1);
    pow1[0] = 1; pow2[0] = 1;
    for (int i = 1; i <= n; ++i) {
        pow1[i] = (pow1[i - 1] * BASE1) % MOD1;
        pow2[i] = (pow2[i - 1] * BASE2) % MOD2;
    }
    for (int i = 0; i < n; ++i) {
        uint64_t v = (uint64_t)(s[i] - 'a' + 1);
        pref1[i + 1] = (pref1[i] * BASE1 + v) % MOD1;
        pref2[i + 1] = (pref2[i] * BASE2 + v) % MOD2;
    }

    unordered_set<uint64_t> seen;
    seen.reserve((size_t)n * n / 2);
    for (int l = 0; l < n; ++l) {
        for (int len = 2; l + len <= n; len += 2) {
            int mid = l + len / 2;
            int r = l + len - 1;
            uint32_t h1a = getHash(pref1, pow1, MOD1, l, mid - 1);
            uint32_t h1b = getHash(pref1, pow1, MOD1, mid, r);
            if (h1a != h1b) continue;
            uint32_t h2a = getHash(pref2, pow2, MOD2, l, mid - 1);
            uint32_t h2b = getHash(pref2, pow2, MOD2, mid, r);
            if (h2a != h2b) continue;

            uint32_t hs1 = getHash(pref1, pow1, MOD1, l, r);
            uint32_t hs2 = getHash(pref2, pow2, MOD2, l, r);
            uint64_t key = (uint64_t(hs1) << 32) ^ uint64_t(hs2);
            seen.insert(key);
        }
    }
    return (int)seen.size();
}

int main() {
    vector<string> tests = {
        "abcabcabc",
        "leetcodeleetcode",
        "aaaa",
        "abab",
        "xyz"
    };
    for (auto& t : tests) {
        cout << countSquareDistinctSubstrings(t) << "\n";
    }
    return 0;
}