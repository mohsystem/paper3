#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <climits>

using namespace std;

// This is a mock implementation for testing purposes.
class Master {
private:
    string secret;
    unordered_set<string> wordSet;
    int guesses;
    int allowedGuesses;
    bool found;

    int countMatches(const string& w1, const string& w2) {
        int matches = 0;
        for (size_t i = 0; i < w1.length(); ++i) {
            if (w1[i] == w2[i]) {
                matches++;
            }
        }
        return matches;
    }

public:
    Master(string secret, const vector<string>& words, int allowedGuesses) {
        this->secret = secret;
        this->wordSet.insert(words.begin(), words.end());
        this->allowedGuesses = allowedGuesses;
        this->guesses = 0;
        this->found = false;
    }

    int guess(string word) {
        cout << "Guessing: " << word << endl;
        guesses++;
        if (guesses > allowedGuesses) {
            cout << "Exceeded allowed guesses!" << endl;
        }
        if (wordSet.find(word) == wordSet.end()) {
            return -1;
        }
        if (word == secret) {
            found = true;
        }
        return countMatches(secret, word);
    }

    void printResult() {
        if (found && guesses <= allowedGuesses) {
            cout << "You guessed the secret word correctly." << endl;
        } else {
            cout << "Either you took too many guesses, or you did not find the secret word." << endl;
        }
    }
};

class Task194 {
private:
    int countMatches(const string& w1, const string& w2) {
        int matches = 0;
        for (size_t i = 0; i < w1.length(); ++i) {
            if (w1[i] == w2[i]) {
                matches++;
            }
        }
        return matches;
    }

public:
    void findSecretWord(vector<string>& words, Master& master) {
        vector<string> candidates = words;

        for (int i = 0; i < 30; ++i) { // Max allowed guesses from constraints
            if (candidates.empty()) break;
            
            string bestGuess;
            if (candidates.size() <= 2) {
                bestGuess = candidates[0];
            } else {
                int minMaxPartition = INT_MAX;
                for (const string& guessWord : words) {
                    vector<int> partitions(7, 0);
                    for (const string& candidateWord : candidates) {
                        partitions[countMatches(guessWord, candidateWord)]++;
                    }
                    int maxPartition = 0;
                    for (int count : partitions) {
                        maxPartition = max(maxPartition, count);
                    }
                    if (maxPartition < minMaxPartition) {
                        minMaxPartition = maxPartition;
                        bestGuess = guessWord;
                    }
                }
            }
            
            int matches = master.guess(bestGuess);
            if (matches == 6) {
                return;
            }

            vector<string> nextCandidates;
            for (const string& candidate : candidates) {
                if (countMatches(bestGuess, candidate) == matches) {
                    nextCandidates.push_back(candidate);
                }
            }
            candidates = nextCandidates;
        }
    }
};

int main() {
    Task194 solution;

    // Test Case 1
    cout << "--- Test Case 1 ---" << endl;
    vector<string> words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
    Master master1("acckzz", words1, 10);
    solution.findSecretWord(words1, master1);
    master1.printResult();
    
    // Test Case 2
    cout << "\n--- Test Case 2 ---" << endl;
    vector<string> words2 = {"hamada","khaled"};
    Master master2("hamada", words2, 10);
    solution.findSecretWord(words2, master2);
    master2.printResult();
    
    // Test Case 3
    cout << "\n--- Test Case 3 ---" << endl;
    vector<string> words3 = {"wichbx","oahwep","tpulot","eqznrt","vjhszz","pcmatp","xqmsgr",
                           "bnrhml","ccoyyo","ajcwbj","arwfnl","nafmtm","xoaumd","vbejda",
                           "kaefne","swcrkh","reeyhj","vmcwaf","chxitv","qkwjna","vklpkp",
                           "xfnayl","ktgmfn","xrmzzm","fgtuki","zcffuv","srxuus","pydgmq"};
    Master master3("ccoyyo", words3, 15);
    solution.findSecretWord(words3, master3);
    master3.printResult();
    
    // Test Case 4
    cout << "\n--- Test Case 4 ---" << endl;
    vector<string> words4 = {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff"};
    Master master4("aaaaaa", words4, 10);
    solution.findSecretWord(words4, master4);
    master4.printResult();
    
    // Test Case 5
    cout << "\n--- Test Case 5 ---" << endl;
    vector<string> words5 = {"abacac", "abadaf", "adafac", "afacab"};
    Master master5("abacac", words5, 10);
    solution.findSecretWord(words5, master5);
    master5.printResult();
    
    return 0;
}