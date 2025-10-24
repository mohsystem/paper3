#include <bits/stdc++.h>
using namespace std;

string getJsonRootType(const string& s) {
    size_t i = 0, n = s.size();
    while (i < n && isspace(static_cast<unsigned char>(s[i]))) i++;
    if (i >= n) return "invalid";
    char c = s[i];
    if (c == '{') return "object";
    if (c == '[') return "array";
    if (c == '"') return "string";
    if (c == 't' || c == 'f') return "boolean";
    if (c == 'n') return "null";
    if (c == '-' || (c >= '0' && c <= '9')) return "number";
    return "invalid";
}

int main() {
    vector<string> tests = {
        "{\"name\":\"Alice\",\"age\":30}",
        "[{\"id\":1},{\"id\":2}]",
        "\"sample\"",
        "-42.0e10",
        "true"
    };
    for (auto& t : tests) {
        cout << getJsonRootType(t) << "\n";
    }
    return 0;
}