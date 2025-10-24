#include <bits/stdc++.h>
using namespace std;

class Master {
    unordered_set<string> wordset;
    string secret;
    int allowed;
    int calls;
    bool found;
public:
    Master(const vector<string>& words, const string& secret, int allowedGuesses)
        : secret(secret), allowed(allowedGuesses), calls(0), found(false) {
        for (auto &w : words) wordset.insert(w);
    }
    int guess(const string& word) {
        if (!wordset.count(word)) return -1;
        calls++;
        int m = 0;
        for (int i = 0; i < 6; ++i) if (word[i] == secret[i]) m++;
        if (m == 6) found = true;
        return m;
    }
    int getCalls() const { return calls; }
    bool isFound() const { return found; }
    int getAllowed() const { return allowed; }
};

static int matches(const string& a, const string& b) {
    int c = 0;
    for (int i = 0; i < 6; ++i) if (a[i] == b[i]) c++;
    return c;
}

static string pickBest(const vector<string>& candidates) {
    int n = (int)candidates.size();
    int bestIdx = 0;
    int bestWorst = INT_MAX;
    for (int i = 0; i < n; ++i) {
        int buckets[7] = {0};
        for (int j = 0; j < n; ++j) {
            int m = matches(candidates[i], candidates[j]);
            buckets[m]++;
        }
        int worst = 0;
        for (int b = 0; b <= 6; ++b) worst = max(worst, buckets[b]);
        if (worst < bestWorst) {
            bestWorst = worst;
            bestIdx = i;
        }
    }
    return candidates[bestIdx];
}

string guessSecret(const vector<string>& words, const string& secret, int allowedGuesses) {
    Master master(words, secret, allowedGuesses);
    vector<string> candidates = words;
    while (!candidates.empty() && !master.isFound() && master.getCalls() < master.getAllowed()) {
        string g = pickBest(candidates);
        int res = master.guess(g);
        if (res == 6) break;
        vector<string> next;
        for (auto &w : candidates) {
            if (w != g && matches(w, g) == res) next.push_back(w);
        }
        candidates.swap(next);
    }
    if (master.isFound() && master.getCalls() <= master.getAllowed()) {
        return "You guessed the secret word correctly.";
    } else {
        return "Either you took too many guesses, or you did not find the secret word.";
    }
}

int main() {
    // Test case 1
    vector<string> words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
    cout << guessSecret(words1, "acckzz", 10) << "\n";
    // Test case 2
    vector<string> words2 = {"hamada","khaled"};
    cout << guessSecret(words2, "hamada", 10) << "\n";
    // Test case 3
    vector<string> words3 = {"python","typhon","thynop","cython","pebble","little","button","people","planet","plenty"};
    cout << guessSecret(words3, "python", 10) << "\n";
    // Test case 4
    vector<string> words4 = {"planet","planer","planed","planes","platen","palnet","placet","planar"};
    cout << guessSecret(words4, "planet", 10) << "\n";
    // Test case 5
    vector<string> words5 = {"silver","salver","sliver","sailor","slider","silken","sinker","sifter","sinner","single"};
    cout << guessSecret(words5, "silver", 10) << "\n";
    return 0;
}