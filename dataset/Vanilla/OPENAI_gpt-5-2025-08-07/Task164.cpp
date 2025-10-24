#include <bits/stdc++.h>
using namespace std;

static string trim(const string& s) {
    size_t start = s.find_first_not_of(" \t\n\r\f\v");
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r\f\v");
    return s.substr(start, end - start + 1);
}

static string lastToken(const string& s) {
    string t = trim(s);
    if (t.empty()) return "";
    size_t pos = t.find_last_of(" \t\n\r\f\v");
    if (pos == string::npos) return t;
    return t.substr(pos + 1);
}

static string toLower(string s) {
    for (char& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

vector<string> lastNameLensort(const vector<string>& names) {
    vector<string> arr = names;
    sort(arr.begin(), arr.end(), [](const string& a, const string& b) {
        string la = lastToken(a);
        string lb = lastToken(b);
        if (la.size() != lb.size()) return la.size() < lb.size();
        string laL = toLower(la), lbL = toLower(lb);
        if (laL != lbL) return laL < lbL;
        return toLower(a) < toLower(b);
    });
    return arr;
}

static void printVec(const vector<string>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        cout << "\"" << v[i] << "\"";
        if (i + 1 < v.size()) cout << ", ";
    }
    cout << "]\n";
}

int main() {
    vector<string> t1 = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    vector<string> t2 = {
        "John Doe",
        "Alice Roe",
        "Bob Poe",
        "Zed Kay"
    };
    vector<string> t3 = {
        "Anna Smith",
        "Brian Smith",
        "Aaron Smith",
        "Zoe Adams"
    };
    vector<string> t4 = {
        "Al A",
        "Bea Bee",
        "Ce Ce",
        "Dee Eee",
        "Eff Eff"
    };
    vector<string> t5 = {
        "Mary Ann Van Dyke",
        "Jean-Claude Van Damme",
        "Jo Van",
        "Li Wei",
        "X AE A-12 Musk"
    };

    printVec(lastNameLensort(t1));
    printVec(lastNameLensort(t2));
    printVec(lastNameLensort(t3));
    printVec(lastNameLensort(t4));
    printVec(lastNameLensort(t5));
    return 0;
}