#include <iostream>
#include <string>
#include <vector>
using namespace std;

string longest(const string& s1, const string& s2) {
    bool present[26] = {false};
    for (char c : s1) {
        if (c >= 'a' && c <= 'z') present[c - 'a'] = true;
    }
    for (char c : s2) {
        if (c >= 'a' && c <= 'z') present[c - 'a'] = true;
    }
    string res;
    res.reserve(26);
    for (int i = 0; i < 26; ++i) {
        if (present[i]) res.push_back(char('a' + i));
    }
    return res;
}

int main() {
    vector<pair<string, string>> tests = {
        {"xyaabbbccccdefww", "xxxxyyyyabklmopq"},
        {"abcdefghijklmnopqrstuvwxyz", "abcdefghijklmnopqrstuvwxyz"},
        {"", ""},
        {"aaabbb", "bbbccc"},
        {"hello", "world"}
    };
    for (auto& t : tests) {
        cout << longest(t.first, t.second) << '\n';
    }
    return 0;
}