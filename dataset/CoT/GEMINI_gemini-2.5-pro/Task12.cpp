#include <iostream>
#include <string>
#include <sstream>
#include <vector>

/**
 * @brief Calculates the score of a word based on alphabet positions.
 * a=1, b=2, etc.
 * @param word The word to score.
 * @return The integer score of the word.
 */
int getScore(const std::string& word) {
    int score = 0;
    for (char c : word) {
        score += c - 'a' + 1;
    }
    return score;
}

/**
 * @brief Finds the highest scoring word in a string.
 * If scores are tied, the word appearing earliest is returned.
 * @param s The input string of words.
 * @return The highest scoring word.
 */
std::string highestScoringWord(const std::string& s) {
    std::stringstream ss(s);
    std::string word;
    std::string highestWord = "";
    int maxScore = -1;

    while (ss >> word) {
        int currentScore = getScore(word);
        if (currentScore > maxScore) {
            maxScore = currentScore;
            highestWord = word;
        }
    }
    return highestWord;
}

int main() {
    // Test cases
    std::vector<std::string> testCases = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "aa b",
        "b aa"
    };

    std::vector<std::string> expectedResults = {
        "taxi",
        "volcano",
        "semynak",
        "aa",
        "b"
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::string result = highestScoringWord(testCases[i]);
        std::cout << "Input: \"" << testCases[i] << "\"" << std::endl;
        std::cout << "Output: " << result << std::endl;
        std::cout << "Expected: " << expectedResults[i] << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}