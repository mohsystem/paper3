#include <bits/stdc++.h>
using namespace std;

static const long long MOD1 = 1000000007LL;
static const long long MOD2 = 1000000009LL;
static const long long BASE1 = 911382323LL;
static const long long BASE2 = 972663749LL;

struct Key {
    uint32_t h1;
    uint32_t h2;
    uint32_t len;
    bool operator==(const Key& o) const {
        return h1 == o.h1 && h2 == o.h2 && len == o.len;
    }
};

struct KeyHasher {
    size_t operator()(const Key& k) const noexcept {
        uint64_t x = (uint64_t)k.h1 * 1315423911u ^ ((uint64_t)k.h2 << 13) ^ k.len;
        x ^= (x >> 33);
        x *= 0xff51afd7ed558ccdULL;
        x ^= (x >> 33);
        x *= 0xc4ceb9fe1a85ec53ULL;
        x ^= (x >> 33);
        return (size_t)x;
    }
};

static inline uint32_t subHash(const vector<long long>& pref, const vector<long long>& pw, int l, int r, long long mod) {
    long long res = (pref[r + 1] - (pref[l] * pw[r - l + 1]) % mod);
    if (res < 0) res += mod;
    return (uint32_t)res;
}

int countEvenSquareDistinct(const string& text) {
    int n = (int)text.size();
    if (n <= 1) return 0;

    vector<long long> pref1(n + 1, 0), pref2(n + 1, 0);
    vector<long long> pow1(n + 1, 1), pow2(n + 1, 1);

    for (int i = 0; i < n; ++i) {
        int v = (text[i] - 'a' + 1);
        pref1[i + 1] = (pref1[i] * BASE1 + v) % MOD1;
        pref2[i + 1] = (pref2[i] * BASE2 + v) % MOD2;
        pow1[i + 1] = (pow1[i] * BASE1) % MOD1;
        pow2[i + 1] = (pow2[i] * BASE2) % MOD2;
    }

    unordered_set<Key, KeyHasher> seen;
    seen.reserve((size_t)n * 2);

    for (int len = 2; len <= n; len += 2) {
        int half = len / 2;
        for (int i = 0; i + len <= n; ++i) {
            int mid = i + half;
            int j = i + len - 1;
            uint32_t a1 = subHash(pref1, pow1, i, mid - 1, MOD1);
            uint32_t b1 = subHash(pref1, pow1, mid, j, MOD1);
            if (a1 != b1) continue;
            uint32_t a2 = subHash(pref2, pow2, i, mid - 1, MOD2);
            uint32_t b2 = subHash(pref2, pow2, mid, j, MOD2);
            if (a2 != b2) continue;
            Key k;
            k.h1 = subHash(pref1, pow1, i, j, MOD1);
            k.h2 = subHash(pref2, pow2, i, j, MOD2);
            k.len = (uint32_t)len;
            seen.insert(k);
        }
    }
    return (int)seen.size();
}

int main() {
    vector<string> tests = {
        "abcabcabc",
        "leetcodeleetcode",
        "aaaa",
        "ababa",
        "z"
    };
    for (const auto& s : tests) {
        cout << countEvenSquareDistinct(s) << "\n";
    }
    return 0;
}