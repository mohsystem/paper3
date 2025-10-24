#include <bits/stdc++.h>
using namespace std;

class Task173Helper {
public:
    static string longestDupSubstring(const string& s) {
        int n = (int)s.size();
        if (n <= 1) return "";
        const long long MOD1 = 1000000007LL;
        const long long MOD2 = 1000000009LL;
        const long long BASE1 = 911382323LL % MOD1;
        const long long BASE2 = 972663749LL % MOD2;

        vector<int> vals(n);
        for (int i = 0; i < n; ++i) vals[i] = (s[i] - 'a' + 1);

        vector<long long> pow1(n + 1, 1), pow2(n + 1, 1), pref1(n + 1, 0), pref2(n + 1, 0);
        for (int i = 0; i < n; ++i) {
            pow1[i + 1] = (pow1[i] * BASE1) % MOD1;
            pow2[i + 1] = (pow2[i] * BASE2) % MOD2;
            pref1[i + 1] = (pref1[i] * BASE1 + vals[i]) % MOD1;
            pref2[i + 1] = (pref2[i] * BASE2 + vals[i]) % MOD2;
        }

        auto getHash = [&](const vector<long long>& pref, const vector<long long>& pow, long long mod, int i, int L) -> long long {
            long long res = (pref[i + L] - (pref[i] * pow[L]) % mod + mod) % mod;
            return res;
        };

        auto check = [&](int L) -> int {
            if (L == 0) return 0;
            unordered_map<long long, int> seen;
            seen.reserve(n * 2);
            for (int i = 0; i + L <= n; ++i) {
                long long h1 = getHash(pref1, pow1, MOD1, i, L);
                long long h2 = getHash(pref2, pow2, MOD2, i, L);
                long long key = (h1 << 32) ^ h2;
                auto it = seen.find(key);
                if (it != seen.end()) return i;
                seen.emplace(key, i);
            }
            return -1;
        };

        int lo = 1, hi = n - 1, bestLen = 0, bestIdx = -1;
        while (lo <= hi) {
            int mid = (lo + hi) / 2;
            int idx = check(mid);
            if (idx != -1) {
                bestLen = mid;
                bestIdx = idx;
                lo = mid + 1;
            } else {
                hi = mid - 1;
            }
        }
        if (bestIdx == -1) return "";
        return s.substr(bestIdx, bestLen);
    }
};

int main() {
    vector<string> tests = {"banana", "abcd", "aaaaa", "abcabc", "mississippi"};
    for (auto& t : tests) {
        cout << "Input: " << t << "\n";
        cout << "Output: " << Task173Helper::longestDupSubstring(t) << "\n\n";
    }
    return 0;
}