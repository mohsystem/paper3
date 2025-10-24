#include <bits/stdc++.h>
using namespace std;

class Task119 {
public:
    static vector<vector<string>> parse_csv(const string& input) {
        vector<vector<string>> rows;
        string field;
        vector<string> row;
        bool inQuotes = false;
        bool anyChar = false;
        bool endedOnNewline = false;

        size_t i = 0, n = input.size();
        while (i < n) {
            char c = input[i];
            anyChar = true;
            if (inQuotes) {
                if (c == '"') {
                    if (i + 1 < n && input[i + 1] == '"') {
                        field.push_back('"');
                        i += 2;
                    } else {
                        inQuotes = false;
                        i++;
                    }
                } else {
                    field.push_back(c);
                    i++;
                }
                endedOnNewline = false;
            } else {
                if (c == '"') {
                    inQuotes = true;
                    i++;
                    endedOnNewline = false;
                } else if (c == ',') {
                    row.push_back(field);
                    field.clear();
                    i++;
                    endedOnNewline = false;
                } else if (c == '\r' || c == '\n') {
                    row.push_back(field);
                    field.clear();
                    rows.push_back(row);
                    row.clear();
                    if (c == '\r' && i + 1 < n && input[i + 1] == '\n') i += 2;
                    else i++;
                    endedOnNewline = true;
                } else {
                    field.push_back(c);
                    i++;
                    endedOnNewline = false;
                }
            }
        }
        if (inQuotes || !field.empty() || !row.empty() || (anyChar && !endedOnNewline)) {
            row.push_back(field);
            rows.push_back(row);
        }
        return rows;
    }

    static string to_csv(const vector<vector<string>>& rows) {
        string out;
        for (size_t r = 0; r < rows.size(); r++) {
            const auto& row = rows[r];
            for (size_t c = 0; c < row.size(); c++) {
                const string& f = row[c];
                bool needQuotes = false;
                for (char ch : f) {
                    if (ch == '"' || ch == ',' || ch == '\n' || ch == '\r') {
                        needQuotes = true;
                        break;
                    }
                }
                if (needQuotes) {
                    out.push_back('"');
                    for (char ch : f) {
                        if (ch == '"') out += "\"\"";
                        else out.push_back(ch);
                    }
                    out.push_back('"');
                } else {
                    out += f;
                }
                if (c + 1 < row.size()) out.push_back(',');
            }
            if (r + 1 < rows.size()) out.push_back('\n');
        }
        return out;
    }
};

static void print_rows(const string& title, const vector<vector<string>>& rows) {
    cout << title << "\n";
    for (size_t i = 0; i < rows.size(); i++) {
        cout << "Row " << i << " [";
        for (size_t j = 0; j < rows[i].size(); j++) {
            if (j) cout << " | ";
            cout << rows[i][j];
        }
        cout << "]\n";
    }
}

int main() {
    vector<string> tests = {
        "a,b,c\n1,2,3",
        "name,quote\nJohn,\"Hello, world\"",
        "id,notes\n1,\"Line1\nLine2\"\n2,\"A\"",
        "text\n\"He said \"\"Hi\"\"\"",
        "a,b,c\r\n1,,3\r\n,2,\r\n"
    };
    for (size_t t = 0; t < tests.size(); t++) {
        cout << "==== Test " << (t + 1) << " ====\n";
        auto parsed = Task119::parse_csv(tests[t]);
        print_rows("Parsed:", parsed);
        string ser = Task119::to_csv(parsed);
        cout << "Serialized:\n" << ser << "\n\n";
    }
    return 0;
}