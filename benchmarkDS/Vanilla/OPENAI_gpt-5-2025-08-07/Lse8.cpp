#include <bits/stdc++.h>
using namespace std;

string valueAtIndexFromArg1(const vector<string>& argv) {
    if (argv.size() < 2) return string();
    try {
        size_t pos = 0;
        int idx = stoi(argv[1], &pos);
        if (pos != argv[1].size()) return string();
        if (idx >= 0 && idx < static_cast<int>(argv.size())) {
            return argv[idx];
        }
        return string();
    } catch (...) {
        return string();
    }
}

int main() {
    vector<vector<string>> tests = {
        {"prog", "2", "a", "b", "c"},
        {"prog", "0", "X"},
        {"prog", "4", "w", "x", "y", "z"},
        {"prog", "1", "hello"},
        {"prog", "3", "p", "q", "r", "s"}
    };
    for (const auto& t : tests) {
        string res = valueAtIndexFromArg1(t);
        cout << res << endl;
    }
    return 0;
}