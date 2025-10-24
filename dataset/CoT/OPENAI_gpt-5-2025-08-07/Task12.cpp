// Step 1: Problem understanding
// Implement function to find the highest scoring word based on letter positions.
// Step 2: Security requirements
// Use safe parsing and bounds-checked operations. Avoid undefined behavior.
// Step 3: Secure coding generation
// Step 4: Code review
// Step 5: Secure code output

#include <bits/stdc++.h>
using namespace std;

string highestScoringWord(const string& s) {
    if (s.empty()) return "";
    istringstream iss(s);
    string word;
    string bestWord;
    long long bestScore = LLONG_MIN;
    while (iss >> word) {
        long long score = 0;
        for (char ch : word) {
            if (ch >= 'a' && ch <= 'z') {
                score += (ch - 'a' + 1);
            }
        }
        if (score > bestScore) {
            bestScore = score;
            bestWord = word;
        }
    }
    return bestWord;
}

int main() {
    // 5 test cases
    vector<string> tests = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b",
        "zoo aaaa"
    };
    for (const auto& t : tests) {
        cout << highestScoringWord(t) << "\n";
    }
    return 0;
}