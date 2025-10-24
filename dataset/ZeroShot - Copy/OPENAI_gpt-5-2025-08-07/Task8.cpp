#include <bits/stdc++.h>
using namespace std;

char findMissingLetter(const vector<char>& arr) {
    if (arr.size() < 2) {
        throw invalid_argument("Input array must have at least 2 characters.");
    }
    auto is_letter = [](char c){ return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); };
    bool isLower = islower(static_cast<unsigned char>(arr[0]));
    bool isUpper = isupper(static_cast<unsigned char>(arr[0]));
    if (!isLower && !isUpper) throw invalid_argument("Input must contain alphabetic characters.");
    for (char c : arr) {
        if (!is_letter(c)) throw invalid_argument("Input must contain only letters.");
        if ((isLower && !islower(static_cast<unsigned char>(c))) ||
            (isUpper && !isupper(static_cast<unsigned char>(c)))) {
            throw invalid_argument("All letters must be in the same case.");
        }
    }
    int expected = static_cast<unsigned char>(arr[0]) + 1;
    for (size_t i = 1; i < arr.size(); ++i) {
        if (static_cast<int>(static_cast<unsigned char>(arr[i])) != expected) {
            return static_cast<char>(expected);
        }
        ++expected;
    }
    throw invalid_argument("No missing letter found.");
}

int main() {
    vector<vector<char>> tests = {
        {'a','b','c','d','f'}, // -> 'e'
        {'O','Q','R','S'},     // -> 'P'
        {'m','n','p','q'},     // -> 'o'
        {'A','B','C','E'},     // -> 'D'
        {'t','v'}              // -> 'u'
    };
    for (const auto& t : tests) {
        char missing = findMissingLetter(t);
        cout << "Missing: '" << missing << "'" << "\n";
    }
    return 0;
}