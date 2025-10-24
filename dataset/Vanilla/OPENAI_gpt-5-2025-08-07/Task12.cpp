#include <bits/stdc++.h>
using namespace std;

string high(const string& s) {
    int bestScore = -1, currScore = 0;
    int bestStart = -1, bestLen = 0;
    int currStart = -1, currLen = 0;

    auto finishWord = [&]() {
        if (currLen > 0) {
            if (currScore > bestScore) {
                bestScore = currScore;
                bestStart = currStart;
                bestLen = currLen;
            }
        }
        currScore = 0;
        currLen = 0;
        currStart = -1;
    };

    for (int i = 0; i <= (int)s.size(); ++i) {
        char ch = (i < (int)s.size() ? s[i] : ' ');
        if (ch >= 'a' && ch <= 'z') {
            if (currLen == 0) currStart = i;
            currLen++;
            currScore += (ch - 'a' + 1);
        } else if (ch == ' ') {
            finishWord();
        } else {
            // Ignore unexpected characters; not expected per problem
            finishWord();
        }
    }

    if (bestStart == -1) return string();
    return s.substr(bestStart, bestLen);
}

int main() {
    vector<string> tests = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b",
        "b aa"
    };
    for (const auto& t : tests) {
        cout << high(t) << "\n";
    }
    return 0;
}