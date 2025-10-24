#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
#include <numeric>
#include <algorithm>
#include <limits>

// This is the Master's API interface.
// You should not implement it, or speculate about its implementation.
class Master {
public:
    virtual int guess(const std::string& word) = 0;
    virtual ~Master() = default;
};

// For testing purposes, we create a concrete implementation of the Master API.
class MasterImpl : public Master {
private:
    std::string secret;
    std::unordered_set<std::string> wordSet;
    int guessesMade;
    int allowedGuesses;
    bool found;

public:
    MasterImpl(const std::string& s, const std::vector<std::string>& words, int ag)
        : secret(s), wordSet(words.begin(), words.end()), guessesMade(0), allowedGuesses(ag), found(false) {}

    int guess(const std::string& word) override {
        guessesMade++;
        if (wordSet.find(word) == wordSet.end()) {
            return -1;
        }
        if (word == secret) {
            found = true;
        }
        return countMatches(secret, word);
    }
    
    std::string getResult() const {
        if (found && guessesMade <= allowedGuesses) {
            return "You guessed the secret word correctly.";
        } else {
            return "Either you took too many guesses, or you did not find the secret word.";
        }
    }
private:
    static int countMatches(const std::string& w1, const std::string& w2) {
        int matches = 0;
        for (size_t i = 0; i < w1.length(); ++i) {
            if (w1[i] == w2[i]) {
                matches++;
            }
        }
        return matches;
    }
};

class Solution {
private:
    int countMatches(const std::string& w1, const std::string& w2) {
        int matches = 0;
        for (size_t i = 0; i < w1.length(); ++i) {
            if (w1[i] == w2[i]) {
                matches++;
            }
        }
        return matches;
    }

public:
    void findSecretWord(const std::vector<std::string>& words, Master& master, int allowedGuesses) {
        std::vector<std::string> candidates = words;

        for (int i = 0; i < allowedGuesses && !candidates.empty(); ++i) {
            std::string bestGuess = "";
            int minMaxGroupSize = std::numeric_limits<int>::max();

            for (const auto& w1 : words) {
                std::vector<int> groups(7, 0); // 0-6 matches
                for (const auto& w2 : candidates) {
                    groups[countMatches(w1, w2)]++;
                }

                int maxGroupSize = 0;
                for (int size : groups) {
                    if (size > maxGroupSize) {
                        maxGroupSize = size;
                    }
                }
                
                if (maxGroupSize < minMaxGroupSize) {
                    minMaxGroupSize = maxGroupSize;
                    bestGuess = w1;
                }
            }

            int matches = master.guess(bestGuess);
            if (matches == 6) {
                return;
            }

            std::vector<std::string> nextCandidates;
            for (const auto& candidate : candidates) {
                if (countMatches(bestGuess, candidate) == matches) {
                    nextCandidates.push_back(candidate);
                }
            }
            candidates = nextCandidates;
        }
    }
};

void run_test(const std::string& test_name, const std::vector<std::string>& words, const std::string& secret, int allowedGuesses) {
    Solution solver;
    MasterImpl master(secret, words, allowedGuesses);
    solver.findSecretWord(words, master, allowedGuesses);
    std::cout << test_name << ": " << master.getResult() << std::endl;
}

int main() {
    // Test Case 1
    std::vector<std::string> words1 = {"acckzz", "ccbazz", "eiowzz", "abcczz"};
    run_test("Test Case 1", words1, "acckzz", 10);

    // Test Case 2
    std::vector<std::string> words2 = {"hamada", "khaled"};
    run_test("Test Case 2", words2, "hamada", 10);

    // Test Case 3
    std::vector<std::string> words3 = {"aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff"};
    run_test("Test Case 3", words3, "aaaaaa", 10);

    // Test Case 4
    std::vector<std::string> words4 = {"abacab", "abacba", "ababca", "ababac", "ababab", "bababa", "babacb", "bababc", "bacaba", "bacbab"};
    run_test("Test Case 4", words4, "abacab", 10);

    // Test Case 5
    std::vector<std::string> words5 = {"abcdef", "zzzzzz", "aaaaaa", "bbbbbb", "cccccc", "dddddd", "eeeeee", "ffffff", "gggggg", "hhhhhh"};
    run_test("Test Case 5", words5, "zzzzzz", 15);

    return 0;
}