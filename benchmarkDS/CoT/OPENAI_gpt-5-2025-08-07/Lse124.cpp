#include <bits/stdc++.h>
using namespace std;

static inline string rtrim_copy(const string& s) {
    if (s.empty()) return s;
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    if (end == string::npos) return string();
    return s.substr(0, end + 1);
}

// Function: accepts input as parameter and returns outputs
pair<string, string> processInput(const string& input) {
    return {input, rtrim_copy(input)};
}

// Command-line behavior as described
void runWithArgs(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Error: exactly one argument required." << endl;
        exit(1);
    }
    string input = argv[1];
    cout << input << endl;
    string trimmed = rtrim_copy(input);
    cout << trimmed << endl;
    exit(0);
}

int main(int argc, char* argv[]) {
    // 5 test cases
    vector<string> tests = {
        "hello world   ",
        "   leading and trailing\t\t",
        "no-trailing",
        "tabs-and-newline\t\t\n",
        ""
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        auto res = processInput(tests[i]);
        cout << "TEST" << (i + 1) << " original: [" << res.first << "]" << endl;
        cout << "TEST" << (i + 1) << " trimmed: [" << res.second << "]" << endl;
    }
    // If any CLI args are provided, run the specified behavior
    if (argc > 1) {
        runWithArgs(argc, argv);
    }
    return 0;
}