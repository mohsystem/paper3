#include <bits/stdc++.h>
using namespace std;

// Returns any duplicated substring with maximum length, or empty string if none.
string longestDupSubstring(const string& s) {
    int n = (int)s.size();
    if (n < 2) return "";
    const long long MOD1 = 1000000007LL;
    const long long MOD2 = 1000000009LL;
    const long long BASE = 911382323LL;

    vector<long long> pow1(n+1, 1), pow2(n+1, 1), pre1(n+1, 0), pre2(n+1, 0);
    for (int i = 0; i < n; ++i) {
        int v = (s[i] - 'a' + 1);
        pow1[i+1] = (pow1[i] * BASE) % MOD1;
        pow2[i+1] = (pow2[i] * BASE) % MOD2;
        pre1[i+1] = (pre1[i] * BASE + v) % MOD1;
        pre2[i+1] = (pre2[i] * BASE + v) % MOD2;
    }

    auto keyAt = [&](int l, int L) -> uint64_t {
        int r = l + L;
        long long h1 = (pre1[r] - (pre1[l] * pow1[L]) % MOD1);
        if (h1 < 0) h1 += MOD1;
        long long h2 = (pre2[r] - (pre2[l] * pow2[L]) % MOD2);
        if (h2 < 0) h2 += MOD2;
        return (uint64_t(h1) << 32) ^ uint64_t(h2);
    };

    auto check = [&](int L) -> int {
        if (L == 0) return 0;
        unordered_map<uint64_t, vector<int>> mp;
        mp.reserve(n * 2);
        for (int i = 0; i + L <= n; ++i) {
            uint64_t k = keyAt(i, L);
            auto &vec = mp[k];
            for (int j : vec) {
                if (s.compare(i, L, s, j, L) == 0) return i;
            }
            vec.push_back(i);
        }
        return -1;
    };

    int lo = 1, hi = n - 1, bestLen = 0, bestStart = -1;
    while (lo <= hi) {
        int mid = lo + ((hi - lo) >> 1);
        int pos = check(mid);
        if (pos != -1) {
            bestLen = mid;
            bestStart = pos;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }
    if (bestLen == 0) return "";
    return s.substr(bestStart, bestLen);
}

int main() {
    vector<string> tests = {
        "banana",
        "abcd",
        "aaaaa",
        "abcdabc",
        "mississippi"
    };
    for (auto &t : tests) {
        cout << "Input: " << t << " -> Output: " << longestDupSubstring(t) << "\n";
    }
    return 0;
}