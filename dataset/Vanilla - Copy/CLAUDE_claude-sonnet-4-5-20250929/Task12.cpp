
#include <iostream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

string highestScoringWord(string s) {
    stringstream ss(s);
    string word;
    string highestWord = "";
    int highestScore = 0;
    
    while (ss >> word) {
        int score = 0;
        for (char c : word) {
            score += c - 'a' + 1;
        }
        if (score > highestScore) {
            highestScore = score;
            highestWord = word;
        }
    }
    
    return highestWord;
}

int main() {
    // Test case 1
    cout << highestScoringWord("man i need a taxi up to ubud") << endl;
    
    // Test case 2
    cout << highestScoringWord("what time are we climbing up the volcano") << endl;
    
    // Test case 3
    cout << highestScoringWord("take me to semynak") << endl;
    
    // Test case 4
    cout << highestScoringWord("aa b") << endl;
    
    // Test case 5
    cout << highestScoringWord("abad") << endl;
    
    return 0;
}
