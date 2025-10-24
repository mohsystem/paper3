#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Function to calculate the score of a word based on alphabet positions.
int calculateScore(const std::string& word) {
    int score = 0;
    for (char c : word) {
        if (c >= 'a' && c <= 'z') {
            score += c - 'a' + 1;
        }
    }
    return score;
}

// Function to find the highest scoring word in a string.
// If scores are tied, the word that appears first is returned.
std::string highestScoringWord(const std::string& str) {
    std::stringstream ss(str);
    std::string word;
    std::string highestWord;
    int maxScore = -1;

    while (ss >> word) {
        int currentScore = calculateScore(word);
        if (currentScore > maxScore) {
            maxScore = currentScore;
            highestWord = word;
        }
    }

    return highestWord;
}

int main() {
    // Test Case 1
    std::string input1 = "man i need a taxi up to ubud";
    std::cout << "Input: \"" << input1 << "\"\nHighest scoring word: " << highestScoringWord(input1) << std::endl << std::endl;

    // Test Case 2
    std::string input2 = "what time are we climbing up the volcano";
    std::cout << "Input: \"" << input2 << "\"\nHighest scoring word: " << highestScoringWord(input2) << std::endl << std::endl;

    // Test Case 3
    std::string input3 = "take me to semynak";
    std::cout << "Input: \"" << input3 << "\"\nHighest scoring word: " << highestScoringWord(input3) << std::endl << std::endl;

    // Test Case 4: Tie, 'b' (2) vs 'aa' (2). 'aa' appears first.
    std::string input4 = "aa b";
    std::cout << "Input: \"" << input4 << "\"\nHighest scoring word: " << highestScoringWord(input4) << std::endl << std::endl;

    // Test Case 5: Tie, 'b' (2) vs 'aa' (2). 'b' appears first.
    std::string input5 = "b aa";
    std::cout << "Input: \"" << input5 << "\"\nHighest scoring word: " << highestScoringWord(input5) << std::endl << std::endl;

    return 0;
}