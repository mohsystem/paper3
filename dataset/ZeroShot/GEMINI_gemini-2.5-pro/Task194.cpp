#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <limits>

// This is a TestMaster class to simulate the problem's environment for local testing.
class Master {
private:
    std::string secret;
    std::unordered_set<std::string> wordSet;
    int allowedGuesses;
    int guessesMade;
    bool found;

    int match(const std::string& a, const std::string& b) {
        int matches = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            if (a[i] == b[i]) {
                matches++;
            }
        }
        return matches;
    }

public:
    Master(std::string s, const std::vector<std::string>& words, int ag) 
        : secret(s), allowedGuesses(ag), guessesMade(0), found(false) {
        wordSet.insert(words.begin(), words.end());
    }

    int guess(const std::string& word) {
        guessesMade++;
        if (wordSet.find(word) == wordSet.end()) {
            return -1;
        }
        int matches = match(word, secret);
        if (matches == 6) {
            found = true;
        }
        return matches;
    }
    
    void report() {
        if (found && guessesMade <= allowedGuesses) {
            std::cout << "You guessed the secret word correctly." << std::endl;
        } else {
            std::cout << "Either you took too many guesses, or you did not find the secret word." << std::endl;
        }
        std::cout << "Guesses made: " << guessesMade << std::endl;
    }
};

class Task194 {
public:
    int match(const std::string& a, const std::string& b) {
        int matches = 0;
        for (size_t i = 0; i < a.length(); ++i) {
            if (a[i] == b[i]) {
                matches++;
            }
        }
        return matches;
    }

    void findSecretWord(std::vector<std::string>& words, Master& master) {
        std::vector<std::string> candidates = words;
        
        // The problem constraints state allowedGuesses is at most 30.
        for (int i = 0; i < 30; ++i) {
            if (candidates.empty()) break;
            
            // Minimax strategy: Choose the best word to guess from the current candidates.
            std::string bestGuess = candidates[0];
            int minMaxGroupSize = std::numeric_limits<int>::max();

            for (const auto& w1 : candidates) {
                std::vector<int> groups(7, 0); // groups[i] stores count of words with i matches
                for (const auto& w2 : candidates) {
                    groups[match(w1, w2)]++;
                }
                
                int maxGroup = 0;
                for (int size : groups) {
                    if (size > maxGroup) {
                        maxGroup = size;
                    }
                }
                
                if (maxGroup < minMaxGroupSize) {
                    minMaxGroupSize = maxGroup;
                    bestGuess = w1;
                }
            }

            int matches = master.guess(bestGuess);
            if (matches == 6) {
                return;
            }

            // Prune the candidate list.
            std::vector<std::string> nextCandidates;
            for (const auto& word : candidates) {
                if (match(bestGuess, word) == matches) {
                    nextCandidates.push_back(word);
                }
            }
            candidates = nextCandidates;
        }
    }
};

int main() {
    Task194 solver;

    // Test Case 1
    std::cout << "Test Case 1:" << std::endl;
    std::vector<std::string> words1 = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
    Master master1("acckzz", words1, 10);
    solver.findSecretWord(words1, master1);
    master1.report();

    // Test Case 2
    std::cout << "\nTest Case 2:" << std::endl;
    std::vector<std::string> words2 = {"hamada", "khaled"};
    Master master2("hamada", words2, 10);
    solver.findSecretWord(words2, master2);
    master2.report();

    // Test Case 3
    std::cout << "\nTest Case 3:" << std::endl;
    std::vector<std::string> words3 = {"wichbx", "oahwep", "tpulot", "eqznzs", "vvmplb", "eywinm"};
    Master master3("oahwep", words3, 10);
    solver.findSecretWord(words3, master3);
    master3.report();

    // Test Case 4
    std::cout << "\nTest Case 4:" << std::endl;
    std::vector<std::string> words4 = {"hbaczn", "aqqbxu", "ajasjw", "rdexkw", "fcceuc", "pcblmb", "bcckqc", "ccbzzw", "agagxt", "ccbazy"};
    Master master4("hbaczn", words4, 10);
    solver.findSecretWord(words4, master4);
    master4.report();

    // Test Case 5
    std::cout << "\nTest Case 5:" << std::endl;
    std::vector<std::string> words5 = {"aaaaaa", "bbbbbb", "cccccc"};
    Master master5("bbbbbb", words5, 10);
    solver.findSecretWord(words5, master5);
    master5.report();

    return 0;
}