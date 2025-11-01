#include <bits/stdc++.h>
using namespace std;

struct Result {
    string text;
    int count;
    size_t consumed;
};

Result scan(const string& input) {
    Result r;
    r.count = 0;
    r.consumed = 0;
    r.text.reserve(1023);
    for (size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if (ch == '\n') {
            r.consumed += 1;
            break;
        }
        if (r.count < 1023) {
            r.text.push_back(ch);
            r.count += 1;
        }
        r.consumed += 1;
    }
    return r;
}

int main() {
    vector<string> tests;
    tests.push_back("hello\nworld");                // normal line with newline
    tests.push_back("\nstart");                     // newline first
    tests.push_back(string(1100, 'a') + "\nNEXT");  // longer than 1023, then newline
    tests.push_back("no newline here");             // no newline
    tests.push_back("");                            // empty input

    for (size_t i = 0; i < tests.size(); ++i) {
        Result r = scan(tests[i]);
        cout << "Test " << (i+1) << ": text=\"" << r.text << "\", count=" << r.count
             << ", consumed=" << r.consumed << "\n";
    }
    return 0;
}