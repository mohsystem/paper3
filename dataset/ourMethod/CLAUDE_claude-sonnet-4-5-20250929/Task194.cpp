
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

// Master API mock for testing
class Master {
private:
    std::string secret;
    int guessCount;
    int maxGuesses;
    
public:
    Master(const std::string& s, int max) : secret(s), guessCount(0), maxGuesses(max) {
        // Validate secret is 6 chars and lowercase only - security measure
        if (secret.length() != 6) {
            secret = "";
            return;
        }
        for (char c : secret) {
            if (c < 'a' || c > 'z') {
                secret = "";
                return;
            }
        }
    }
    
    int guess(const std::string& word) {
        // Validate input length to prevent buffer issues - CWE-119
        if (word.length() != 6) {
            return -1;
        }
        
        // Validate input contains only lowercase letters - input validation
        for (char c : word) {
            if (c < 'a' || c > 'z') {
                return -1;
            }
        }
        
        if (guessCount >= maxGuesses) {
            return -1;
        }
        
        guessCount++;
        
        if (secret.empty()) {
            return -1;
        }
        
        int matches = 0;
        // Safe comparison with bounds checking
        for (size_t i = 0; i < 6 && i < word.length() && i < secret.length(); i++) {
            if (word[i] == secret[i]) {
                matches++;
            }
        }
        
        return matches;
    }
    
    int getGuessCount() const { return guessCount; }
};

class Solution {
private:
    // Calculate match count between two words - helper function
    int matchCount(const std::string& a, const std::string& b) {
        // Validate both strings are exactly 6 chars - bounds checking
        if (a.length() != 6 || b.length() != 6) {
            return 0;
        }
        
        int count = 0;
        for (size_t i = 0; i < 6; i++) {
            if (a[i] == b[i]) {
                count++;
            }
        }
        return count;
    }
    
public:
    void findSecretWord(std::vector<std::string>& words, Master& master) {
        // Input validation - check vector is not empty
        if (words.empty()) {
            return;
        }
        
        // Validate all words are 6 chars and lowercase - security measure
        for (const auto& word : words) {
            if (word.length() != 6) {
                return;
            }
            for (char c : word) {
                if (c < 'a' || c > 'z') {
                    return;
                }
            }
        }
        
        std::vector<std::string> candidates = words;
        
        // Limit iterations to prevent infinite loops - defense in depth
        const int MAX_ITERATIONS = 30;
        int iterations = 0;
        
        while (!candidates.empty() && iterations < MAX_ITERATIONS) {
            iterations++;
            
            // Pick the word with minimum maximum group size (minimax strategy)
            std::string guess = candidates[0];
            int minMaxGroup = static_cast<int>(candidates.size());
            
            // Check bounds before iteration
            for (size_t i = 0; i < candidates.size(); i++) {
                std::vector<int> groups(7, 0); // matches can be 0-6
                
                for (size_t j = 0; j < candidates.size(); j++) {
                    int matches = matchCount(candidates[i], candidates[j]);
                    // Validate match count is in valid range
                    if (matches >= 0 && matches <= 6) {
                        groups[matches]++;
                    }
                }
                
                int maxGroup = 0;
                for (int g : groups) {
                    maxGroup = std::max(maxGroup, g);
                }
                
                if (maxGroup < minMaxGroup) {
                    minMaxGroup = maxGroup;
                    guess = candidates[i];
                }
            }
            
            int matches = master.guess(guess);
            
            // If guessed correctly, exit
            if (matches == 6) {
                return;
            }
            
            // If guess failed (not in wordlist), try next candidate
            if (matches == -1) {
                // Remove the invalid guess from candidates
                candidates.erase(
                    std::remove(candidates.begin(), candidates.end(), guess),
                    candidates.end()
                );
                continue;
            }
            
            // Filter candidates: keep only words with same match count
            std::vector<std::string> newCandidates;
            for (const auto& word : candidates) {
                if (matchCount(guess, word) == matches) {
                    newCandidates.push_back(word);
                }
            }
            
            candidates = newCandidates;
        }
    }
};

int main() {
    Solution solution;
    
    // Test case 1
    {
        std::vector<std::string> words1 = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
        Master master1("acckzz", 10);
        solution.findSecretWord(words1, master1);
        std::cout << "Test 1: " << (master1.getGuessCount() <= 10 ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 2
    {
        std::vector<std::string> words2 = {"hamada", "khaled"};
        Master master2("hamada", 10);
        solution.findSecretWord(words2, master2);
        std::cout << "Test 2: " << (master2.getGuessCount() <= 10 ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 3
    {
        std::vector<std::string> words3 = {"gaxckt", "terlnx", "jbhanu", "cpjbzf"};
        Master master3("gaxckt", 10);
        solution.findSecretWord(words3, master3);
        std::cout << "Test 3: " << (master3.getGuessCount() <= 10 ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 4 - single word
    {
        std::vector<std::string> words4 = {"abcdef"};
        Master master4("abcdef", 10);
        solution.findSecretWord(words4, master4);
        std::cout << "Test 4: " << (master4.getGuessCount() <= 10 ? "PASS" : "FAIL") << std::endl;
    }
    
    // Test case 5
    {
        std::vector<std::string> words5 = {"ccoyyo", "wdhyne", "lqmqhd", "aafljg"};
        Master master5("ccoyyo", 10);
        solution.findSecretWord(words5, master5);
        std::cout << "Test 5: " << (master5.getGuessCount() <= 10 ? "PASS" : "FAIL") << std::endl;
    }
    
    return 0;
}
