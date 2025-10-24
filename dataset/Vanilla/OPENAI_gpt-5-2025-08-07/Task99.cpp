#include <bits/stdc++.h>
using namespace std;

vector<string> matchLines(const string& filePath) {
    vector<string> result;
    regex pattern("^\\d+.*\\.$");
    ifstream in(filePath);
    if (!in.is_open()) return result;
    string line;
    while (std::getline(in, line)) {
        // Remove trailing \r if present (CRLF)
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (regex_match(line, pattern)) {
            result.push_back(line);
        }
    }
    return result;
}

int main() {
    // Test case 1
    string f1 = "cpp_test1.txt";
    {
        ofstream out(f1);
        out << "123 Start here.\n";
        out << "Not starting with number.\n";
        out << "42 Another line.\n";
        out << "7 ends with no dot\n";
        out << "0.\n";
    }
    auto r1 = matchLines(f1);
    cout << "Test 1 matches:";
    for (auto& s : r1) cout << " [" << s << "]";
    cout << "\n";

    // Test case 2
    string f2 = "cpp_test2.txt";
    {
        ofstream out(f2);
        out << "10 Leading digits not ending with dot\n";
        out << "20 ends with dot.\n";
        out << ". starts with dot.\n";
        out << "99Trailing spaces.\n";
    }
    auto r2 = matchLines(f2);
    cout << "Test 2 matches:";
    for (auto& s : r2) cout << " [" << s << "]";
    cout << "\n";

    // Test case 3
    string f3 = "cpp_test3.txt";
    {
        ofstream out(f3);
        out << "No numbers at start.\n";
        out << "3.14159.\n";
        out << "31415\n";
        out << "2718.\n";
    }
    auto r3 = matchLines(f3);
    cout << "Test 3 matches:";
    for (auto& s : r3) cout << " [" << s << "]";
    cout << "\n";

    // Test case 4
    string f4 = "cpp_test4.txt";
    {
        ofstream out(f4);
        out << "A line.\n";
        out << "123.\n";
        out << "abc123.\n";
        out << "456 trailing spaces .\n";
        out << "789!.\n";
    }
    auto r4 = matchLines(f4);
    cout << "Test 4 matches:";
    for (auto& s : r4) cout << " [" << s << "]";
    cout << "\n";

    // Test case 5 (CRLF endings)
    string f5 = "cpp_test5.txt";
    {
        ofstream out(f5, ios::binary);
        out << "100 First line.\r\n";
        out << "no number.\r\n";
        out << "200 End with dot.\r\n";
        out << "300 no dot\r\n";
        out << "400.\r\n";
    }
    auto r5 = matchLines(f5);
    cout << "Test 5 matches:";
    for (auto& s : r5) cout << " [" << s << "]";
    cout << "\n";

    return 0;
}