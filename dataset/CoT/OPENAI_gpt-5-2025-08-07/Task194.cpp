#include <bits/stdc++.h>
using namespace std;

/*
Chain-of-Through process:
1) Understand: Implement a solver to find a 6-letter secret by querying Master.guess within a limit.
2) Security: Validate inputs, prevent out-of-bounds, avoid undefined behavior.
3) Secure coding: Use standard containers, check lengths, sanitize parameters.
4) Review: Minimax strategy for efficient narrowing; no excessive copies; bounded loops.
5) Output: Return only specified messages.
*/

class Master {
    unordered_set<string> dict;
    string secret;
    int allowed;
    int calls;
    bool success;
public:
    Master(const vector<string>& words, const string& sec, int allow): secret(sec), allowed(max(0, allow)), calls(0), success(false) {
        for (const auto& w: words) dict.insert(w);
        if (!dict.count(secret)) throw invalid_argument("Secret not in words");
    }
    int guess(const string& word) {
        if (word.size()!=6 || !dict.count(word)) return -1;
        calls++;
        int m=0; for (int i=0;i<6;i++) if (word[i]==secret[i]) m++;
        if (m==6) success = true;
        return m;
    }
    string resultMessage() const {
        if (success && calls <= allowed) return "You guessed the secret word correctly.";
        return "Either you took too many guesses, or you did not find the secret word.";
    }
};

static inline int matchCount(const string& a, const string& b) {
    int m=0; for (int i=0;i<6;i++) if (a[i]==b[i]) m++; return m;
}

static string chooseGuess(const vector<string>& cands) {
    string best = cands[0];
    int bestScore = INT_MAX;
    for (const auto& w : cands) {
        int bucket[7] = {0,0,0,0,0,0,0};
        for (const auto& v : cands) {
            bucket[matchCount(w, v)]++;
        }
        int worst = 0;
        for (int k=0;k<7;k++) worst = max(worst, bucket[k]);
        if (worst < bestScore) {
            bestScore = worst;
            best = w;
        }
    }
    return best;
}

string solve(vector<string> words, const string& secret, int allowedGuesses) {
    if (secret.size()!=6) return "Either you took too many guesses, or you did not find the secret word.";
    vector<string> wl;
    wl.reserve(words.size());
    for (auto &w: words) if (w.size()==6) wl.push_back(w);
    Master master(wl, secret, allowedGuesses);
    vector<string> cands = wl;

    for (int g=0; g<allowedGuesses && !cands.empty(); g++) {
        string guess = chooseGuess(cands);
        int res = master.guess(guess);
        if (res == 6) break;
        vector<string> next;
        next.reserve(cands.size());
        for (auto &w : cands) {
            if (matchCount(w, guess) == res) next.push_back(w);
        }
        cands.swap(next);
    }
    return master.resultMessage();
}

int main() {
    vector<vector<string>> testsWords = {
        {"acckzz","ccbazz","eiowzz","abcczz"},
        {"hamada","khaled"},
        {"bbbbbb"},
        {"kitten","kittey","bitten","sitten","mitten","kittzz"},
        {"acbdef","ghijkl","monkey","abcdef","zzzzzz","qwerty","yellow","hammer","flower","planet"}
    };
    vector<string> testsSecret = {"acckzz","hamada","bbbbbb","mitten","flower"};
    vector<int> testsAllowed = {10,10,1,10,20};

    for (size_t i=0;i<testsSecret.size();i++) {
        cout << solve(testsWords[i], testsSecret[i], testsAllowed[i]) << "\n";
    }
    return 0;
}