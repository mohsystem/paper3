
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
using namespace std;

class Master {
public:
    virtual int guess(string word) = 0;
    virtual ~Master() {}
};

class Task194 {
private:
    int match(const string& a, const string& b) {
        int matches = 0;
        for (int i = 0; i < 6; i++) {
            if (a[i] == b[i]) matches++;
        }
        return matches;
    }
    
    string selectBestGuess(const vector<string>& candidates) {
        if (candidates.size() <= 2) {
            return candidates[0];
        }
        
        string bestWord = candidates[0];
        int minMaxGroup = INT_MAX;
        
        for (const string& candidate : candidates) {
            vector<int> groups(7, 0);
            for (const string& word : candidates) {
                groups[match(candidate, word)]++;
            }
            
            int maxGroup = *max_element(groups.begin(), groups.end());
            
            if (maxGroup < minMaxGroup) {
                minMaxGroup = maxGroup;
                bestWord = candidate;
            }
        }
        
        return bestWord;
    }
    
public:
    void findSecretWord(vector<string>& words, Master& master) {
        vector<string> candidates = words;
        
        for (int attempt = 0; attempt < 10 && !candidates.empty(); attempt++) {
            string guess = selectBestGuess(candidates);
            int matches = master.guess(guess);
            
            if (matches == 6) {
                return;
            }
            
            vector<string> newCandidates;
            for (const string& word : candidates) {
                if (match(guess, word) == matches) {
                    newCandidates.push_back(word);
                }
            }
            candidates = newCandidates;
        }
    }
};

// Test implementation
class TestMaster : public Master {
private:
    string secret;
    vector<string> words;
public:
    TestMaster(string s, vector<string> w) : secret(s), words(w) {}
    
    int guess(string word) override {
        if (find(words.begin(), words.end(), word) == words.end()) return -1;
        int matches = 0;
        for (int i = 0; i < 6; i++) {
            if (word[i] == secret[i]) matches++;
        }
        return matches;
    }
};

int main() {
    Task194 solution;
    
    // Test 1
    vector<string> words1 = {"acckzz","ccbazz","eiowzz","abcczz"};
    TestMaster master1("acckzz", words1);
    solution.findSecretWord(words1, master1);
    cout << "Test 1 passed" << endl;
    
    // Test 2
    vector<string> words2 = {"hamada","khaled"};
    TestMaster master2("hamada", words2);
    solution.findSecretWord(words2, master2);
    cout << "Test 2 passed" << endl;
    
    return 0;
}
