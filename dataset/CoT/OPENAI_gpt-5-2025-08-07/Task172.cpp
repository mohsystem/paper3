#include <bits/stdc++.h>
using namespace std;

struct Key {
    uint64_t h1;
    uint64_t h2;
    int len;
    bool operator==(const Key& o) const {
        return h1 == o.h1 && h2 == o.h2 && len == o.len;
    }
};

struct KeyHasher {
    size_t operator()(const Key& k) const noexcept {
        uint64_t a = k.h1 ^ (k.h1 >> 33);
        uint64_t b = k.h2 ^ (k.h2 >> 29);
        uint64_t c = (a * 0x9e3779b97f4a7c15ULL) ^ (b + 0x9e3779b97f4a7c15ULL + (uint64_t)k.len);
        return (size_t)(c ^ (c >> 32));
    }
};

static const long long MOD1 = 1000000007LL;
static const long long MOD2 = 1000000009LL;
static const long long BASE1 = 911382323LL;
static const long long BASE2 = 972663749LL;

struct RollingHash {
    vector<long long> h1, h2, p1, p2;
    RollingHash(const string& s) {
        int n = (int)s.size();
        h1.assign(n + 1, 0);
        h2.assign(n + 1, 0);
        p1.assign(n + 1, 1);
        p2.assign(n + 1, 1);
        for (int i = 0; i < n; ++i) {
            int v = s[i] - 'a' + 1;
            h1[i + 1] = ( (h1[i] * BASE1) % MOD1 + v ) % MOD1;
            h2[i + 1] = ( (h2[i] * BASE2) % MOD2 + v ) % MOD2;
            p1[i + 1] = (p1[i] * BASE1) % MOD1;
            p2[i + 1] = (p2[i] * BASE2) % MOD2;
        }
    }
    pair<long long,long long> get(int l, int r) const { // [l, r)
        long long x1 = (h1[r] - (h1[l] * p1[r - l]) % MOD1);
        if (x1 < 0) x1 += MOD1;
        long long x2 = (h2[r] - (h2[l] * p2[r - l]) % MOD2);
        if (x2 < 0) x2 += MOD2;
        return {x1, x2};
    }
};

int countSquareSubstrings(const string& text) {
    int n = (int)text.size();
    if (n <= 1) return 0;
    RollingHash rh(text);
    unordered_set<Key, KeyHasher> st;
    for (int len = 2; len <= n; len += 2) {
        for (int i = 0; i + len <= n; ++i) {
            int mid = i + len / 2;
            auto L = rh.get(i, mid);
            auto R = rh.get(mid, i + len);
            if (L.first == R.first && L.second == R.second) {
                auto F = rh.get(i, i + len);
                st.insert(Key{(uint64_t)F.first, (uint64_t)F.second, len});
            }
        }
    }
    return (int)st.size();
}

int main() {
    vector<string> tests = {
        "abcabcabc",
        "leetcodeleetcode",
        "aaaaaa",
        "abcd",
        "abbaabba"
    };
    for (auto &t : tests) {
        cout << countSquareSubstrings(t) << "\n";
    }
    return 0;
}