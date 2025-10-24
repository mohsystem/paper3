// Step 1-5: Secure CSV parser with robust handling of quotes, commas, and newlines (RFC 4180-like)
#include <iostream>
#include <vector>
#include <string>

std::vector<std::vector<std::string>> parseCSV(const std::string& content, char delimiter = ',') {
    std::vector<std::vector<std::string>> records;
    if (content.empty()) return records;

    std::vector<std::string> row;
    std::string field;
    bool inQuotes = false;

    for (size_t i = 0; i < content.size(); ++i) {
        char ch = content[i];
        if (inQuotes) {
            if (ch == '"') {
                if (i + 1 < content.size() && content[i + 1] == '"') {
                    field.push_back('"');
                    ++i;
                } else {
                    inQuotes = false;
                }
            } else {
                field.push_back(ch);
            }
        } else {
            if (ch == '"') {
                inQuotes = true;
            } else if (ch == delimiter) {
                row.emplace_back(field);
                field.clear();
            } else if (ch == '\n' || ch == '\r') {
                row.emplace_back(field);
                field.clear();
                records.emplace_back(row);
                row.clear();
                if (ch == '\r' && i + 1 < content.size() && content[i + 1] == '\n') {
                    ++i;
                }
            } else {
                field.push_back(ch);
            }
        }
    }

    if (inQuotes || !field.empty() || !row.empty()) {
        row.emplace_back(field);
        records.emplace_back(row);
    }

    return records;
}

static void printParsed(const std::string& title, const std::vector<std::vector<std::string>>& recs) {
    std::cout << "=== " << title << " ===\n";
    for (size_t i = 0; i < recs.size(); ++i) {
        std::cout << "Row " << i << ": ";
        for (size_t j = 0; j < recs[i].size(); ++j) {
            std::string f = recs[i][j];
            for (char& c : f) {
                if (c == '\n') c = '\n'; // leave; replaced in output below
            }
            // Replace CR and LF for display
            std::string disp;
            for (char c : recs[i][j]) {
                if (c == '\n') { disp += "\\n"; }
                else if (c == '\r') { disp += "\\r"; }
                else { disp.push_back(c); }
            }
            std::cout << "[" << disp << "]";
            if (j + 1 < recs[i].size()) std::cout << " | ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main() {
    std::string t1 = "a,b,c\n1,2,3";
    std::string t2 = "Name,Note\n\"Smith, John\",\"He said \"\"Hello\"\".\"";
    std::string t3 = "A,B\n\"Line1\nLine2\",X";
    std::string t4 = "col1,col2,col3,col4\n, , ,\n,,";
    std::string t5 = "x,y\r\n1,2\r\n3,\"4\r\n5\",6";

    printParsed("Test 1", parseCSV(t1));
    printParsed("Test 2", parseCSV(t2));
    printParsed("Test 3", parseCSV(t3));
    printParsed("Test 4", parseCSV(t4));
    printParsed("Test 5", parseCSV(t5));

    return 0;
}