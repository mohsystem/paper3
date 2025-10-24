#include <bits/stdc++.h>
using namespace std;

// Processes the input string by removing non-whitespace control characters,
// collapsing consecutive whitespace to a single space, and trimming.
string processInput(const string& input) {
    string output;
    output.reserve(input.size());
    bool inSpace = false;

    for (unsigned char uc : input) {
        bool isWhitespace = (uc == ' ' || uc == '\t' || uc == '\n' || uc == '\r' || uc == '\f' || uc == '\v');
        bool isControl = (uc < 32 || uc == 127);
        if (isControl && !isWhitespace) {
            continue; // drop non-whitespace control chars
        }
        if (isWhitespace) {
            if (!output.empty() && !inSpace) {
                output.push_back(' ');
                inSpace = true;
            }
        } else {
            output.push_back(static_cast<char>(uc));
            inSpace = false;
        }
    }

    // Trim trailing space
    if (!output.empty() && output.back() == ' ') {
        output.pop_back();
    }
    return output;
}

int main(int argc, char** argv) {
    // Five test cases
    vector<string> tests = {
        "Hello,  World!",
        "   Leading and trailing   ",
        "Line1\t\tLine2",
        string("Control") + char(7) + "Char and \t tabs",
        "aaaaa     bbbbb     ccccc    "
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << ": [" << processInput(tests[i]) << "]\n";
    }

    // If user provides arguments, treat them as user input
    if (argc > 1) {
        string userInput;
        for (int i = 1; i < argc; ++i) {
            if (i > 1) userInput.push_back(' ');
            userInput += argv[i];
        }
        cout << "User Input: [" << processInput(userInput) << "]\n";
    }

    return 0;
}