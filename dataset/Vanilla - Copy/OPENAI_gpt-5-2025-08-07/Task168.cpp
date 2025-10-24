#include <iostream>
#include <vector>
#include <string>
#include <cctype>
using namespace std;

string tweakLetters(const string& s, const vector<int>& arr) {
    string out = s;
    for (size_t i = 0; i < out.size(); ++i) {
        char ch = tolower(static_cast<unsigned char>(out[i]));
        if (i < arr.size() && ch >= 'a' && ch <= 'z') {
            int pos = ch - 'a';
            pos = (pos + arr[i]) % 26;
            if (pos < 0) pos += 26;
            out[i] = static_cast<char>('a' + pos);
        } else {
            out[i] = ch;
        }
    }
    return out;
}

int main() {
    vector<pair<string, vector<int>>> tests = {
        {"apple", {0, 1, -1, 0, -1}},
        {"many", {0, 0, 0, -1}},
        {"rhino", {1, 1, 1, 1, 1}},
        {"xyz", {1, 1, 1}},
        {"zoo", {1, 0, 0}}
    };
    for (auto& t : tests) {
        cout << tweakLetters(t.first, t.second) << "\n";
    }
    return 0;
}