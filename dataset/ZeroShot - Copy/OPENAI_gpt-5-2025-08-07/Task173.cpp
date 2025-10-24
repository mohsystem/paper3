#include <bits/stdc++.h>
using namespace std;

string longestDupSubstring(const string& s) {
    int n = (int)s.size();
    if (n < 2) return "";
    vector<int> sa = buildSuffixArray(s);
    vector<int> lcp = buildLCP(s, sa);
    int maxL = 0, pos = -1;
    for (int i = 0; i < n - 1; i++) {
        if (lcp[i] > maxL) {
            maxL = lcp[i];
            pos = sa[i];
        }
    }
    if (maxL == 0) return "";
    return s.substr(pos, maxL);
}

static void countingSort(const vector<int>& saIn, const vector<int>& key, int maxKey, vector<int>& saOut) {
    int n = (int)saIn.size();
    vector<int> cnt(maxKey + 1, 0);
    for (int idx : saIn) cnt[key[idx]]++;
    int sum = 0;
    for (int i = 0; i <= maxKey; i++) {
        int t = cnt[i];
        cnt[i] = sum;
        sum += t;
    }
    for (int idx : saIn) {
        int k = key[idx];
        saOut[cnt[k]++] = idx;
    }
}

vector<int> buildSuffixArray(const string& s) {
    int n = (int)s.size();
    vector<int> sa(n), rnk(n), tmp(n), key1(n), key2(n), out(n);
    for (int i = 0; i < n; i++) {
        sa[i] = i;
        rnk[i] = s[i] - 'a';
    }
    for (int k = 1; k < n; k <<= 1) {
        int maxRank = 0;
        for (int v : rnk) if (v > maxRank) maxRank = v;

        for (int i = 0; i < n; i++) key2[i] = (i + k < n ? rnk[i + k] + 1 : 0);
        countingSort(sa, key2, maxRank + 2, out);

        for (int i = 0; i < n; i++) key1[i] = rnk[i] + 1;
        countingSort(out, key1, maxRank + 2, sa);

        tmp[sa[0]] = 0;
        int classes = 1;
        for (int i = 1; i < n; i++) {
            int cur = sa[i], prev = sa[i - 1];
            if (rnk[cur] != rnk[prev] || key2[cur] != key2[prev]) classes++;
            tmp[cur] = classes - 1;
        }
        rnk.swap(tmp);
        if (classes == n) break;
    }
    return sa;
}

vector<int> buildLCP(const string& s, const vector<int>& sa) {
    int n = (int)s.size();
    vector<int> rank(n, 0);
    for (int i = 0; i < n; i++) rank[sa[i]] = i;
    vector<int> lcp(max(0, n - 1), 0);
    int k = 0;
    for (int i = 0; i < n; i++) {
        int r = rank[i];
        if (r == n - 1) {
            k = 0;
            continue;
        }
        int j = sa[r + 1];
        while (i + k < n && j + k < n && s[i + k] == s[j + k]) k++;
        lcp[r] = k;
        if (k > 0) k--;
    }
    return lcp;
}

int main() {
    vector<string> tests = {
        "banana",
        "abcd",
        "aaaaa",
        "abcabca",
        "mississippi"
    };
    for (auto& t : tests) {
        cout << "Input: " << t << " -> Output: " << longestDupSubstring(t) << "\n";
    }
    return 0;
}