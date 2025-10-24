#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <numeric>
#include <algorithm>

using namespace std;

// This is the Master's API interface.
class Master {
public:
    virtual ~Master() {}
    virtual int guess(string word) = 0;
};

// Helper function to calculate matches
int match(const string& a, const string& b) {
    int matches = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        if (a[i] == b[i]) {
            matches++;
        }
    }
    return matches;
}

// Helper Master class for testing purposes
class MasterImpl : public Master {
private:
    string secret;
    unordered_set<string> wordSet;
    int guessCount;
    int allowedGuesses;
    bool solved;

public:
    MasterImpl(const string& s, const vector<string>& words, int ag) 
        : secret(s), guessCount(0), allowedGuesses(ag), solved(false) {
        for (const auto& word : words) {
            wordSet.insert(word);
        }
    }

    int guess(string word) override {
        guessCount++;
        if (wordSet.find(word) == wordSet.end()) {
            return -1;
        }
        int matches = match(secret, word);
        if (matches == secret.length()) {
            solved = true;
        }
        return matches;
    }

    void printResult() {
        if (solved && guessCount <= allowedGuesses) {
            cout << "You guessed the secret word correctly." << endl;
        } else {
            cout << "Either you took too many guesses, or you did not find the secret word." << endl;
        }
    }
};

class Task194 {
public:
    void findSecretWord(vector<string>& words, Master& master, int allowedGuesses) {
        vector<string> candidates = words;

        for (int i = 0; i < allowedGuesses; ++i) {
            if (candidates.empty()) return;

            string best_guess = candidates[0];
            int minMaxGroupSize = candidates.size();

            for (const string& w1 : candidates) {
                vector<int> groups(7, 0);
                for (const string& w2 : candidates) {
                    groups[match(w1, w2)]++;
                }
                
                int maxGroupSize = 0;
                for (int size : groups) {
                    if (size > maxGroupSize) {
                        maxGroupSize = size;
                    }
                }

                if (maxGroupSize < minMaxGroupSize) {
                    minMaxGroupSize = maxGroupSize;
                    best_guess = w1;
                }
            }

            int matches = master.guess(best_guess);
            if (matches == 6) {
                return;
            }

            vector<string> newCandidates;
            for (const string& candidate : candidates) {
                if (match(candidate, best_guess) == matches) {
                    newCandidates.push_back(candidate);
                }
            }
            candidates = newCandidates;
        }
    }
};

int main() {
    Task194 solver;

    // Test Case 1
    cout << "Test Case 1:" << endl;
    vector<string> words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
    string secret1 = "acckzz";
    int allowedGuesses1 = 10;
    MasterImpl master1(secret1, words1, allowedGuesses1);
    solver.findSecretWord(words1, master1, allowedGuesses1);
    master1.printResult();

    // Test Case 2
    cout << "\nTest Case 2:" << endl;
    vector<string> words2 = {"hamada", "khaled"};
    string secret2 = "hamada";
    int allowedGuesses2 = 10;
    MasterImpl master2(secret2, words2, allowedGuesses2);
    solver.findSecretWord(words2, master2, allowedGuesses2);
    master2.printResult();

    // Test Case 3
    cout << "\nTest Case 3:" << endl;
    vector<string> words3 = {"rccqcb","acckzz","ccbazz","eiowzz","abcczz", "hamada", "bobson", "vrtrva"};
    string secret3 = "bobson";
    int allowedGuesses3 = 10;
    MasterImpl master3(secret3, words3, allowedGuesses3);
    solver.findSecretWord(words3, master3, allowedGuesses3);
    master3.printResult();

    // Test Case 4
    cout << "\nTest Case 4:" << endl;
    vector<string> words4 = {"banana","cabana","banaba","bandan","banban","bonbon"};
    string secret4 = "banana";
    int allowedGuesses4 = 15;
    MasterImpl master4(secret4, words4, allowedGuesses4);
    solver.findSecretWord(words4, master4, allowedGuesses4);
    master4.printResult();
    
    // Test Case 5
    cout << "\nTest Case 5:" << endl;
    vector<string> words5 = {"abcdef","abdfed","acbdef","aefdcb","abdcef","abcefd"};
    string secret5 = "abcdef";
    int allowedGuesses5 = 10;
    MasterImpl master5(secret5, words5, allowedGuesses5);
    solver.findSecretWord(words5, master5, allowedGuesses5);
    master5.printResult();

    return 0;
}