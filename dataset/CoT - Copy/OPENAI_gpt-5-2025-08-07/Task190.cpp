#include <bits/stdc++.h>
using namespace std;

static string transpose(const vector<string>& lines) {
    if (lines.empty()) return "";
    vector<vector<string>> matrix;
    int cols = -1;
    for (const auto& line : lines) {
        if (line.empty()) throw invalid_argument("Empty line not allowed");
        // split by one or more spaces
        vector<string> tokens;
        string token;
        bool in_token = false;
        for (size_t i = 0; i < line.size(); ++i) {
            char ch = line[i];
            if (ch == ' ') {
                if (in_token) {
                    tokens.push_back(token);
                    token.clear();
                    in_token = false;
                }
            } else {
                token.push_back(ch);
                in_token = true;
            }
        }
        if (in_token) tokens.push_back(token);
        if (tokens.empty()) throw invalid_argument("Empty line not allowed");
        if (cols == -1) cols = static_cast<int>(tokens.size());
        else if (cols != static_cast<int>(tokens.size())) throw invalid_argument("Inconsistent column count");
        matrix.push_back(move(tokens));
    }

    int rows = static_cast<int>(matrix.size());
    // build output
    string out;
    // precompute length to minimize reallocations
    size_t total_len = 0;
    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            total_len += matrix[r][c].size();
            if (r < rows - 1) total_len += 1; // space
        }
        if (c < cols - 1) total_len += 1; // newline
    }
    out.reserve(total_len);

    for (int c = 0; c < cols; ++c) {
        for (int r = 0; r < rows; ++r) {
            out += matrix[r][c];
            if (r < rows - 1) out.push_back(' ');
        }
        if (c < cols - 1) out.push_back('\n');
    }
    return out;
}

int main() {
    // 1) Example from prompt
    vector<string> tc1 = {
        "name age",
        "alice 21",
        "ryan 30"
    };
    cout << "TC1:\n" << transpose(tc1) << "\n\n";

    // 2) 1x1
    vector<string> tc2 = {"a"};
    cout << "TC2:\n" << transpose(tc2) << "\n\n";

    // 3) 1x3
    vector<string> tc3 = {"x y z"};
    cout << "TC3:\n" << transpose(tc3) << "\n\n";

    // 4) 3x1
    vector<string> tc4 = {"x", "y", "z"};
    cout << "TC4:\n" << transpose(tc4) << "\n\n";

    // 5) 3x3
    vector<string> tc5 = {
        "a b c",
        "d e f",
        "g h i"
    };
    cout << "TC5:\n" << transpose(tc5) << "\n\n";

    return 0;
}