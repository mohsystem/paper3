
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <climits>
#include <iostream>

using namespace std;

class Master {
public:
    virtual int guess(string word) = 0;
};

class Task194 {
private:
    int countMatches(const string& a, const string& b) {
        int matches = 0;
        for (int i = 0; i < a.length(); i++) {
            if (a[i] == b[i]) {
                matches++;
            }
        }
        return matches;
    }
    
public:
    void findSecretWord(vector<string>& words, Master& master) {
        vector<string> candidates = words;
        
        for (int i = 0; i < 10; i++) {
            if (candidates.empty()) break;
            
            string guess = candidates[0];
            int minMaxGroup = INT_MAX;
            
            for (const string& candidate : candidates) {
                unordered_map<int, int> groups;
                for (const string& word : candidates) {
                    int matches = countMatches(candidate, word);
                    groups[matches]++;
                }
                int maxGroup = 0;
                for (auto& p : groups) {
                    maxGroup = max(maxGroup, p.second);
                }
                if (maxGroup < minMaxGroup) {
                    minMaxGroup = maxGroup;
                    guess = candidate;
                }
            }
            
            int matches = master.guess(guess);
            if (matches == 6) return;
            
            vector<string> newCandidates;
            for (const string& word : candidates) {
                if (countMatches(guess, word) == matches) {
                    newCandidates.push_back(word);
                }
            }
            candidates = newCandidates;
        }
    }
};

int main() {
    Task194 solution;
    cout << "Test 1: Expected to find 'acckzz'" << endl;
    cout << "Test 2: Expected to find 'hamada'" << endl;
    return 0;
}
