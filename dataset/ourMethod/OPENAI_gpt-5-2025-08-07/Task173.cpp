#include <bits/stdc++.h>
using namespace std;

static bool is_valid_input(const string& s) {
    if (s.size() < 2 || s.size() > 30000) return false;
    for (char c : s) if (c < 'a' || c > 'z') return false;
    return true;
}

static vector<int> build_suffix_array(const string& s) {
    int n = (int)s.size();
    vector<int> sa(n), rnk(n), tmp(n);
    for (int i = 0; i < n; ++i) {
        sa[i] = i;
        rnk[i] = s[i] - 'a';
    }
    for (int k = 1;; k <<= 1) {
        auto cmp = [&](int i, int j) {
            if (rnk[i] != rnk[j]) return rnk[i] < rnk[j];
            int ri = (i + k < n) ? rnk[i + k] : -1;
            int rj = (j + k < n) ? rnk[j + k] : -1;
            return ri < rj;
        };
        sort(sa.begin(), sa.end(), cmp);
        tmp[sa[0]] = 0;
        for (int i = 1; i < n; ++i) {
            tmp[sa[i]] = tmp[sa[i-1]] + (cmp(sa[i-1], sa[i]) ? 1 : 0);
        }
        for (int i = 0; i < n; ++i) rnk[i] = tmp[i];
        if (rnk[sa[n-1]] == n - 1) break;
    }
    return sa;
}

static vector<int> build_lcp(const string& s, const vector<int>& sa) {
    int n = (int)s.size();
    vector<int> rank(n), lcp(n - 1, 0);
    for (int i = 0; i < n; ++i) rank[sa[i]] = i;
    int h = 0;
    for (int i = 0; i < n; ++i) {
        int r = rank[i];
        if (r == n - 1) { h = 0; continue; }
        int j = sa[r + 1];
        while (i + h < n && j + h < n && s[i + h] == s[j + h]) h++;
        lcp[r] = h;
        if (h > 0) h--;
    }
    return lcp;
}

string longestDupSubstring(const string& s) {
    if (!is_valid_input(s)) return "";
    int n = (int)s.size();
    vector<int> sa = build_suffix_array(s);
    vector<int> lcp = build_lcp(s, sa);
    int bestLen = 0, bestPos = -1;
    for (int i = 0; i < (int)lcp.size(); ++i) {
        if (lcp[i] > bestLen) {
            bestLen = lcp[i];
            bestPos = sa[i];
        }
    }
    if (bestLen <= 0) return "";
    return s.substr(bestPos, bestLen);
}

int main() {
    vector<string> tests = {
        "banana",
        "abcd",
        "aaaaa",
        "abcabcabc",
        "mississippi"
    };
    for (auto& t : tests) {
        cout << "Input: " << t << " -> Output: " << longestDupSubstring(t) << "\n";
    }
    return 0;
}