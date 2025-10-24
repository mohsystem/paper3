#include <bits/stdc++.h>
using namespace std;

struct Master {
    unordered_set<string> wordSet;
    string secret;
    int allowed;
    int calls;
    Master(const vector<string>& words, const string& sec, int alw): secret(sec), allowed(alw), calls(0) {
        for (const auto& w: words) wordSet.insert(w);
        if (!wordSet.count(secret)) throw invalid_argument("Secret not in words");
        if (allowed < 0) throw invalid_argument("allowed < 0");
    }
    int guess(const string& word) {
        calls++;
        if (!wordSet.count(word)) return -1;
        int cnt = 0;
        for (size_t i = 0; i < word.size() && i < secret.size(); ++i) if (word[i] == secret[i]) cnt++;
        return cnt;
    }
};

static int matchCount(const string& a, const string& b) {
    if (a.size() != b.size()) return -1;
    int cnt = 0;
    for (size_t i = 0; i < a.size(); ++i) if (a[i] == b[i]) cnt++;
    return cnt;
}

static string chooseBestGuess(const vector<string>& cands) {
    int n = (int)cands.size();
    int bestZero = INT_MAX;
    string best = cands[0];
    for (int i = 0; i < n; ++i) {
        int zeros = 0;
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            if (matchCount(cands[i], cands[j]) == 0) zeros++;
        }
        if (zeros < bestZero) {
            bestZero = zeros;
            best = cands[i];
        }
    }
    return best;
}

static bool findSecretWord(vector<string> words, Master& master) {
    vector<string> cands = words;
    while (!cands.empty() && master.calls < master.allowed) {
        string g = chooseBestGuess(cands);
        int res = master.guess(g);
        if (res == 6) return true;
        if (res < 0) return false;
        vector<string> next;
        next.reserve(cands.size());
        for (const auto& w : cands) if (matchCount(w, g) == res) next.push_back(w);
        cands.swap(next);
        if (cands.size() == 1 && master.calls < master.allowed) {
            return master.guess(cands[0]) == 6;
        }
    }
    return false;
}

string solveTestCase(const string& secret, const vector<string>& wordsInput, int allowedGuesses) {
    vector<string> words;
    words.reserve(wordsInput.size());
    for (const auto& w : wordsInput) if (w.size() == 6) words.push_back(w);
    Master master(words, secret, allowedGuesses);
    bool guessed = findSecretWord(words, master);
    if (guessed && master.calls <= allowedGuesses) return "You guessed the secret word correctly.";
    return "Either you took too many guesses, or you did not find the secret word.";
}

int main() {
    // Test case 1
    cout << solveTestCase("acckzz", {"acckzz","ccbazz","eiowzz","abcczz"}, 10) << "\n";
    // Test case 2
    cout << solveTestCase("hamada", {"hamada","khaled"}, 10) << "\n";
    // Test case 3
    cout << solveTestCase("sprite", {"spiral","sprint","sprout","sprite","spirit","spring"}, 10) << "\n";
    // Test case 4
    cout << solveTestCase("zenith", {"abacus","absurd","acumen","agenda","zenith","shrimp","zigzag","little","letter","legend"}, 15) << "\n";
    // Test case 5
    cout << solveTestCase("orange", {"orange","banana","tomato","pepper","potato","carrot","onions","radish","celery","garlic"}, 12) << "\n";
    return 0;
}