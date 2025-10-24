#include <bits/stdc++.h>
using namespace std;

class Task190 {
public:
    static string transpose(const string& content) {
        // Normalize CRLF
        string norm;
        norm.reserve(content.size());
        for (char ch : content) if (ch != '\r') norm.push_back(ch);

        vector<vector<string>> rows;
        {
            string line;
            stringstream ss(norm);
            while (getline(ss, line, '\n')) {
                if (line.empty()) continue;
                stringstream ls(line);
                vector<string> parts;
                string tok;
                while (ls >> tok) parts.push_back(tok);
                if (!parts.empty()) rows.push_back(move(parts));
            }
        }
        if (rows.empty()) return string();
        size_t r = rows.size(), c = rows[0].size();

        // Build output
        ostringstream out;
        for (size_t i = 0; i < c; ++i) {
            for (size_t j = 0; j < r; ++j) {
                if (j) out << ' ';
                out << rows[j][i];
            }
            if (i + 1 < c) out << '\n';
        }
        return out.str();
    }
};

int main() {
    vector<string> tests = {
        "name age\nalice 21\nryan 30",
        "a b c",
        "a\na\na",
        "1 2 3\n4 5 6\n7 8 9",
        "w x y z\n1 2 3 4"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << Task190::transpose(tests[i]) << "\n";
        if (i + 1 < tests.size()) cout << "-----\n";
    }
    return 0;
}