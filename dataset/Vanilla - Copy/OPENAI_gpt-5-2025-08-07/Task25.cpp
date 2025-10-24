#include <bits/stdc++.h>
using namespace std;

vector<string> number(const vector<string>& lines) {
    vector<string> res;
    res.reserve(lines.size());
    for (size_t i = 0; i < lines.size(); ++i) {
        res.emplace_back(to_string(i + 1) + ": " + lines[i]);
    }
    return res;
}

static void printVec(const vector<string>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        if (i) cout << ", ";
        cout << '"' << v[i] << '"';
    }
    cout << "]\n";
}

int main() {
    vector<vector<string>> tests = {
        {},
        {"a", "b", "c"},
        {"", "hello"},
        {"foo bar", "baz"},
        {"x"}
    };
    for (const auto& t : tests) {
        printVec(number(t));
    }
    return 0;
}