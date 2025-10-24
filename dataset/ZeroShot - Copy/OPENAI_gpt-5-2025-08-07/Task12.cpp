#include <iostream>
#include <sstream>
#include <string>
#include <climits>

std::string highestScoringWord(const std::string& s) {
    std::istringstream iss(s);
    std::string w;
    long long bestScore = LLONG_MIN;
    std::string bestWord;
    while (iss >> w) {
        long long score = 0;
        for (char ch : w) {
            if (ch >= 'a' && ch <= 'z') {
                score += (ch - 'a' + 1);
            }
        }
        if (score > bestScore) {
            bestScore = score;
            bestWord = w;
        }
    }
    return bestWord;
}

int main() {
    std::string tests[] = {
        "abad aa bb",
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b"
    };
    for (const auto& t : tests) {
        std::cout << highestScoringWord(t) << std::endl;
    }
    return 0;
}