#include <bits/stdc++.h>
using namespace std;

struct Key {
    uint64_t h1;
    uint64_t h2;
    int len;
    bool operator==(const Key& other) const {
        return h1 == other.h1 && h2 == other.h2 && len == other.len;
    }
};

struct KeyHash {
    size_t operator()(const Key& k) const noexcept {
        uint64_t x = k.h1 * 0x9E3779B97F4A7C15ull ^ (k.h2 + 0xC2B2AE3D27D4EB4Full) ^ (uint64_t)k.len * 0x165667B19E3779F9ull;
        x ^= (x >> 30);
        x *= 0xbf58476d1ce4e5b9ull;
        x ^= (x >> 27);
        x *= 0x94d049bb133111ebull;
        x ^= (x >> 31);
        return (size_t)x;
    }
};

static const uint64_t BASE1 = 11400714819323198485ull;
static const uint64_t BASE2 = 14029467366897019727ull;

static void validate(const string& s) {
    if (s.empty() || s.size() > 2000) {
        throw invalid_argument("length out of range");
    }
    for (char c : s) {
        if (c < 'a' || c > 'z') throw invalid_argument("invalid character");
    }
}

struct Hasher {
    vector<uint64_t> pow1, pow2, h1, h2;
    void build(const string& s) {
        int n = (int)s.size();
        pow1.assign(n + 1, 0);
        pow2.assign(n + 1, 0);
        h1.assign(n + 1, 0);
        h2.assign(n + 1, 0);
        pow1[0] = 1;
        pow2[0] = 1;
        for (int i = 1; i <= n; ++i) {
            pow1[i] = pow1[i - 1] * BASE1;
            pow2[i] = pow2[i - 1] * BASE2;
        }
        for (int i = 0; i < n; ++i) {
            uint64_t v = (uint64_t)(s[i] - 'a' + 1);
            h1[i + 1] = h1[i] * BASE1 + v;
            h2[i + 1] = h2[i] * BASE2 + v;
        }
    }
    inline uint64_t get1(int l, int r) const { return h1[r] - h1[l] * pow1[r - l]; }
    inline uint64_t get2(int l, int r) const { return h2[r] - h2[l] * pow2[r - l]; }
};

int countSquareSubstringsDistinct(const string& text) {
    validate(text);
    int n = (int)text.size();
    if (n < 2) return 0;
    Hasher hasher;
    hasher.build(text);
    unordered_set<Key, KeyHash> seen;
    seen.reserve(1 << 20);
    for (int i = 0; i < n; ++i) {
        for (int len = 2; i + len <= n; len += 2) {
            int mid = i + (len >> 1);
            uint64_t a1 = hasher.get1(i, mid);
            uint64_t b1 = hasher.get1(mid, i + len);
            if (a1 != b1) continue;
            uint64_t a2 = hasher.get2(i, mid);
            uint64_t b2 = hasher.get2(mid, i + len);
            if (a2 != b2) continue;
            Key k{hasher.get1(i, i + len), hasher.get2(i, i + len), len};
            seen.insert(k);
        }
    }
    return (int)seen.size();
}

int main() {
    vector<string> tests = {
        "abcabcabc",        // 3
        "leetcodeleetcode", // 2
        "aaaaaa",           // 3
        "ababab",           // 2
        "a"                 // 0
    };
    for (const auto& t : tests) {
        try {
            cout << "Input: " << t << " -> " << countSquareSubstringsDistinct(t) << "\n";
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}