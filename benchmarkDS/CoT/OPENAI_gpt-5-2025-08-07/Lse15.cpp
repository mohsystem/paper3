#include <bits/stdc++.h>
using namespace std;

// Chain-of-Through process:
// 1) Problem understanding: Prompt user for an index, return value from id_sequence at that index.
// 2) Security requirements: Validate input and bounds; handle EOF/invalid input safely.
// 3) Secure coding generation: Use getline + strtol for robust parsing; validate before indexing.
// 4) Code review: Ensure no unchecked access; handle failures with clear return code.
// 5) Secure code output: Final with proper checks and safe I/O.

static const vector<int> ID_SEQUENCE = {101, 202, 303, 404, 505, 606, 707, 808, 909, 1001};

int getValueAtIndex(int indexParam) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    while (true) {
        cout << "Enter index (0.." << (int)ID_SEQUENCE.size() - 1 << "): " << flush;
        string line;
        if (!std::getline(cin, line)) {
            return -1; // EOF or input closed
        }
        // Trim
        auto start = line.find_first_not_of(" \t\r\n");
        auto end = line.find_last_not_of(" \t\r\n");
        if (start == string::npos) {
            cout << "Invalid input: empty. Please enter an integer index.\n";
            continue;
        }
        string trimmed = line.substr(start, end - start + 1);

        char* endptr = nullptr;
        errno = 0;
        long val = strtol(trimmed.c_str(), &endptr, 10);
        if (errno != 0 || endptr == trimmed.c_str() || *endptr != '\0') {
            cout << "Invalid input: not a valid integer. Try again.\n";
            continue;
        }
        if (val < 0 || val >= (long)ID_SEQUENCE.size()) {
            cout << "Index out of bounds. Valid range is 0.." << (int)ID_SEQUENCE.size() - 1 << ". Try again.\n";
            continue;
        }
        return ID_SEQUENCE[(size_t)val];
    }
}

int main() {
    // 5 test cases; function will prompt for index each time
    for (int t = 1; t <= 5; ++t) {
        int value = getValueAtIndex(-1);
        cout << "Test " << t << " -> Value: " << value << "\n";
    }
    return 0;
}