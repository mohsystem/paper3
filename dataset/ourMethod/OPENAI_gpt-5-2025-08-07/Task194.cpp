#include <bits/stdc++.h>
using namespace std;

static const string SUCCESS_MSG = "You guessed the secret word correctly.";
static const string FAILURE_MSG = "Either you took too many guesses, or you did not find the secret word.";

class Master {
public:
    Master(const vector<string>& words, const string& secret, int allowedGuesses)
        : secret_(secret), allowed_(max(0, allowedGuesses)), guessCount_(0), guessed_(false) {
        for (const auto& w : words) {
            if (w.size() == 6) wordSet_.insert(w);
        }
    }

    int guess(const string& word) {
        guessCount_++;
        if (word.size() != 6 || wordSet_.find(word) == wordSet_.end()) {
            return -1;
        }
        int m = 0;
        for (int i = 0; i < 6; ++i) {
            if (word[i] == secret_[i]) m++;
        }
        if (m == 6) guessed_ = true;
        return m;
    }

    int getGuessCount() const { return guessCount_; }

    string resultMessage() const {
        if (guessed_ && guessCount_ <= allowed_) return SUCCESS_MSG;
        return FAILURE_MSG;
    }

private:
    unordered_set<string> wordSet_;
    string secret_;
    int allowed_;
    int guessCount_;
    bool guessed_;
};

static int matchCount(const string& a, const string& b) {
    int m = 0;
    for (int i = 0; i < 6; ++i) if (a[i] == b[i]) m++;
    return m;
}

string solve(vector<string> words, const string& secret, int allowedGuesses) {
    if (secret.size() != 6) return FAILURE_MSG;
    Master master(words, secret, allowedGuesses);
    vector<int> candidates;
    for (int i = 0; i < (int)words.size(); ++i) {
        if (words[i].size() == 6) candidates.push_back(i);
    }

    while (!candidates.empty() && master.getGuessCount() < allowedGuesses) {
        int bestIdx = -1;
        int bestScore = INT_MAX;
        string bestWord;

        for (int idx : candidates) {
            array<int, 7> buckets{};
            buckets.fill(0);
            const string& w = words[idx];
            for (int jdx : candidates) {
                int m = matchCount(w, words[jdx]);
                buckets[m]++;
            }
            int worst = 0;
            for (int c : buckets) worst = max(worst, c);
            if (worst < bestScore || (worst == bestScore && (bestIdx == -1 || words[idx] < bestWord))) {
                bestScore = worst;
                bestIdx = idx;
                bestWord = words[idx];
            }
        }

        if (bestIdx == -1) break;
        string guessWord = words[bestIdx];
        int res = master.guess(guessWord);
        if (res == 6) break;
        if (res < 0) {
            vector<int> next;
            for (int idx : candidates) if (idx != bestIdx) next.push_back(idx);
            candidates.swap(next);
            continue;
        }
        vector<int> filtered;
        for (int idx : candidates) if (matchCount(words[idx], guessWord) == res) filtered.push_back(idx);
        candidates.swap(filtered);
    }

    return master.resultMessage();
}

int main() {
    vector<vector<string>> testsWords = {
        {"acckzz", "ccbazz", "eiowzz", "abcczz"},
        {"hamada", "khaled"},
        {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeffee", "ffeeee", "fedcba", "abcdef"},
        {"absdef", "abqdef", "abzdef", "abzcef", "abzcdx", "abzcdz", "abzcdy", "abztdy", "qbztdy", "xyzabc"},
        {"nearer", "bearer", "hearer", "seared", "neared", "dealer", "leader", "linear"}
    };
    vector<string> testsSecret = {"acckzz", "hamada", "abcdef", "abzcdz", "neared"};
    vector<int> testsAllowed = {10, 10, 10, 10, 12};

    for (size_t i = 0; i < testsWords.size(); ++i) {
        cout << solve(testsWords[i], testsSecret[i], testsAllowed[i]) << "\n";
    }
    return 0;
}