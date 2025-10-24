#include <iostream>
#include <string>
#include <sstream>
#include <vector>

class Task12 {
public:
    static std::string high(const std::string& s) {
        std::stringstream ss(s);
        std::string word;
        std::string highestScoringWord = "";
        int highestScore = 0;

        while (ss >> word) {
            int currentScore = 0;
            for (char c : word) {
                currentScore += c - 'a' + 1;
            }

            if (currentScore > highestScore) {
                highestScore = currentScore;
                highestScoringWord = word;
            }
        }
        return highestScoringWord;
    }
};

int main() {
    std::vector<std::string> testCases = {
        "man i need a taxi up to ubud",
        "what time are we climbing up the volcano",
        "take me to semynak",
        "bb d",
        "d bb"
    };

    for (const auto& test : testCases) {
        std::cout << "Input: \"" << test << "\"" << std::endl;
        std::cout << "Output: \"" << Task12::high(test) << "\"\n" << std::endl;
    }

    return 0;
}