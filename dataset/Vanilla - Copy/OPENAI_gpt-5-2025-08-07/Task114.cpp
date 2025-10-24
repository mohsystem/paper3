#include <bits/stdc++.h>
using namespace std;

string copyString(const string& s) {
    return string(s);
}

string concatenate(const string& a, const string& b) {
    return a + b;
}

string reverseString(const string& s) {
    string r = s;
    reverse(r.begin(), r.end());
    return r;
}

string toUpperString(const string& s) {
    string r = s;
    for (char& c : r) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return r;
}

string removeWhitespace(const string& s) {
    string r;
    r.reserve(s.size());
    for (unsigned char c : s) {
        if (!isspace(c)) r.push_back(static_cast<char>(c));
    }
    return r;
}

int main() {
    string t1 = "Hello";
    string t2a = "Hello";
    string t2b = " World";
    string t3 = "OpenAI";
    string t4 = "Mixed Case 123";
    string t5 = "  a b\tc\nd  ";

    cout << "Test 1 - copyString: " << copyString(t1) << "\n";
    cout << "Test 2 - concatenate: " << concatenate(t2a, t2b) << "\n";
    cout << "Test 3 - reverseString: " << reverseString(t3) << "\n";
    cout << "Test 4 - toUpperString: " << toUpperString(t4) << "\n";
    cout << "Test 5 - removeWhitespace: " << removeWhitespace(t5) << "\n";
    return 0;
}