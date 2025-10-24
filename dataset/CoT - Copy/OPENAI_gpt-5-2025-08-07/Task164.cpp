#include <bits/stdc++.h>
using namespace std;

static string trim(const string& s) {
    size_t start = s.find_first_not_of(' ');
    if (start == string::npos) return "";
    size_t end = s.find_last_not_of(' ');
    return s.substr(start, end - start + 1);
}

static string lastName(const string& name) {
    string t = trim(name);
    size_t pos = t.find_last_of(' ');
    if (pos == string::npos) return t;
    return t.substr(pos + 1);
}

static string toLower(string s) {
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

vector<string> lastNameLensort(const vector<string>& names) {
    vector<string> res = names;
    stable_sort(res.begin(), res.end(), [](const string& a, const string& b) {
        string la = lastName(a);
        string lb = lastName(b);
        if (la.size() != lb.size()) return la.size() < lb.size();
        string laL = toLower(la), lbL = toLower(lb);
        if (laL != lbL) return laL < lbL;
        return la < lb; // deterministic tie-break
    });
    return res;
}

static void printVec(const vector<string>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        cout << (i ? ", " : "") << "\"" << v[i] << "\"";
    }
    cout << "]\n";
}

int main() {
    // Test case 1
    vector<string> t1 = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    printVec(lastNameLensort(t1));

    // Test case 2
    vector<string> t2 = {
        "John Smith",
        "Amy Brown",
        "Zoey Clark",
        "Evan White",
        "Liam Jones"
    };
    printVec(lastNameLensort(t2));

    // Test case 3
    vector<string> t3 = {
        "Alice   van Helsing",
        "Bob de la Cruz",
        "  Carla   Delaney ",
        "Derek O'Neill",
        "Eve   Stone  "
    };
    printVec(lastNameLensort(t3));

    // Test case 4
    vector<string> t4 = { "Solo Person" };
    printVec(lastNameLensort(t4));

    // Test case 5
    vector<string> t5 = {
        "Ann Lee",
        "Ben Lee",
        "Cara Zee",
        "Dan Lee",
        "Eli Zee"
    };
    printVec(lastNameLensort(t5));

    return 0;
}