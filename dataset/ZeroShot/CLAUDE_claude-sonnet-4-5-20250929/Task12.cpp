
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

string highestScoringWord(const string& s) {
    if (s.empty()) {
        return "";
    }
    
    istringstream iss(s);
    string word;
    string highestWord = "";
    int highestScore = 0;
    
    while (iss >> word) {
        int score = calculateScore(word);
        if (score > highestScore) {
            highestScore = score;
            highestWord = word;
        }
    }
    
    return highestWord;
}

int main() {
    // Test case 1
    cout << "Test 1: " << highestScoringWord("man i need a taxi up to ubud") << endl;
    
    // Test case 2
    cout << "Test 2: " << highestScoringWord("what time are we climbing up the volcano") << endl;
    
    // Test case 3
    cout << "Test 3: " << highestScoringWord("take me to semynak") << endl;
    
    // Test case 4
    cout << "Test 4: " << highestScoringWord("aa b") << endl;
    
    // Test case 5
    cout << "Test 5: " << highestScoringWord("abad") << endl;
    
    return 0;
}
