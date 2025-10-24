
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
using namespace std;

class Master {
public:
    virtual int guess(string word) = 0;
};

class Task194 {
public:
    void findSecretWord(vector<string>& words, Master& master) {
        vector<string> candidates = words;
        
        for (int i = 0; i < 10; i++) {
            if (candidates.empty()) break;
            
            string guess = getBestGuess(candidates);
            int matches = master.guess(guess);
            
            if (matches == 6) return;
            
            candidates = filterCandidates(candidates, guess, matches);
        }
    }
    
private:
    string getBestGuess(vector<string>& candidates) {
        string bestWord = candidates[0];
        int minMaxGroup = INT_MAX;
        
        for (const string& word : candidates) {
            vector<int> matchCounts(7, 0);
            for (const string& candidate : candidates) {
                int matches = countMatches(word, candidate);
                matchCounts[matches]++;
            }
            
            int maxGroup = *max_element(matchCounts.begin(), matchCounts.end());
            
            if (maxGroup < minMaxGroup) {
                minMaxGroup = maxGroup;
                bestWord = word;
            }
        }
        
        return bestWord;
    }
    
    vector<string> filterCandidates(vector<string>& candidates, const string& guess, int targetMatches) {
        vector<string> filtered;
        for (const string& word : candidates) {
            if (countMatches(word, guess) == targetMatches) {
                filtered.push_back(word);
            }
        }
        return filtered;
    }
    
    int countMatches(const string& a, const string& b) {
        int count = 0;
        for (int i = 0; i < 6; i++) {
            if (a[i] == b[i]) count++;
        }
        return count;
    }
};

// Test implementation
class TestMaster : public Master {
    string secret;
    vector<string> wordlist;
public:
    TestMaster(string s, vector<string> w) : secret(s), wordlist(w) {}
    int guess(string word) {
        if (find(wordlist.begin(), wordlist.end(), word) == wordlist.end()) return -1;
        int count = 0;
        for (int i = 0; i < 6; i++) {
            if (word[i] == secret[i]) count++;
        }
        return count;
    }
};

int main() {
    Task194 solution;
    
    // Test 1
    vector<string> words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
    TestMaster master1("acckzz", words1);
    solution.findSecretWord(words1, master1);
    cout << "Test 1 completed" << endl;
    
    // Test 2
    vector<string> words2 = {"hamada","khaled"};
    TestMaster master2("hamada", words2);
    solution.findSecretWord(words2, master2);
    cout << "Test 2 completed" << endl;
    
    return 0;
}
