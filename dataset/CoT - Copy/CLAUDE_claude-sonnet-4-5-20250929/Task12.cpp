
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

int calculateScore(const string& word) {
    int score = 0;
    for (char c : word) {
        if (c >= 'a' && c <= 'z') {
            score += (c - 'a' + 1);
        }
    }
    return score;
}

string highestScoringWord(const string& str) {
    if (str.empty()) {
        return "";
    }
    
    istringstream iss(str);
    string word;
    string highestWord = "";
    int maxScore = 0;
    
    while (iss >> word) {
        int score = calculateScore(word);
        if (score > maxScore) {
            maxScore = score;
            highestWord = word;
        }
    }
    
    return highestWord;
}

int main() {
    cout << "Test 1: " << highestScoringWord("man i need a taxi up to ubud") << endl;
    cout << "Test 2: " << highestScoringWord("what time are we climbing up the volcano") << endl;
    cout << "Test 3: " << highestScoringWord("take me to semynak") << endl;
    cout << "Test 4: " << highestScoringWord("aa b") << endl;
    cout << "Test 5: " << highestScoringWord("a z") << endl;
    return 0;
}
