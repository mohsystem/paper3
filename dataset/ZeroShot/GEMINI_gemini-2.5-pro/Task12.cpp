#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <numeric>

std::string highestScoringWord(const std::string& s) {
    std::stringstream ss(s);
    std::string word;
    std::string highestScoringWord = "";
    int maxScore = -1;

    while (ss >> word) {
        int currentScore = 0;
        for (char c : word) {
            currentScore += c - 'a' + 1;
        }

        if (currentScore > maxScore) {
            maxScore = currentScore;
            highestScoringWord = word;
        }
    }
    return highestScoringWord;
}

int main() {
    std::vector<std::string> testCases = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aaa b",
        "b aa"
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << i + 1 << ": \"" << testCases[i] << "\"" << std::endl;
        std::string result = highestScoringWord(testCases[i]);
        std::cout << "Result: " << result << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}