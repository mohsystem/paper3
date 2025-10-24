#include <iostream>
#include <string>
#include <vector>
#include <limits>

static inline int letterScore(char c) {
    if (c >= 'a' && c <= 'z') return (c - 'a' + 1);
    return 0;
}

std::string highestScoringWord(const std::string& s) {
    const size_t n = s.size();
    size_t bestStart = 0;
    size_t bestLen = 0;
    long long bestScore = std::numeric_limits<long long>::min();

    size_t i = 0;
    while (i < n) {
        // Skip spaces
        while (i < n && s[i] == ' ') i++;
        if (i >= n) break;

        // Start of a word
        size_t start = i;
        long long score = 0;
        while (i < n && s[i] != ' ') {
            score += letterScore(s[i]);
            i++;
        }
        size_t len = i - start;

        if (score > bestScore) {
            bestScore = score;
            bestStart = start;
            bestLen = len;
        }
        // If tie, keep earliest (do nothing)
    }

    if (bestScore == std::numeric_limits<long long>::min()) {
        return std::string(); // empty if no words
    }
    return s.substr(bestStart, bestLen);
}

int main() {
    std::vector<std::string> tests = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b",
        "b aa"
    };

    for (const auto& t : tests) {
        std::string res = highestScoringWord(t);
        std::cout << "Input: \"" << t << "\" -> Highest: \"" << res << "\"\n";
    }
    return 0;
}