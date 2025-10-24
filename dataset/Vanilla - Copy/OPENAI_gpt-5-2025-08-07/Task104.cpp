#include <bits/stdc++.h>
using namespace std;

string handleInput(const string& input, size_t capacity) {
    if (capacity == 0) return string();
    size_t n = min(capacity, input.size());
    return input.substr(0, n);
}

int main() {
    vector<pair<string, size_t>> tests = {
        {"Hello World", 5},
        {"", 10},
        {"Short", 10},
        {"ExactSize", 9},
        {"BoundaryTest", 0}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        string out = handleInput(tests[i].first, tests[i].second);
        cout << "Case " << (i + 1) << ": [" << out << "]\n";
    }
    return 0;
}