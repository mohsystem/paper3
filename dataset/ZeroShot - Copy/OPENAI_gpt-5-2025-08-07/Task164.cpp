#include <bits/stdc++.h>
using namespace std;

static inline string to_lower_copy(const string &s) {
    string r(s.size(), '\0');
    transform(s.begin(), s.end(), r.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
    return r;
}

static inline string extract_last_name(const string &name) {
    // Trim trailing whitespace
    size_t end = name.find_last_not_of(" \t\n\r\f\v");
    if (end == string::npos) return "";
    // Find start of last token
    size_t i = end + 1;
    while (i > 0) {
        unsigned char c = static_cast<unsigned char>(name[i-1]);
        if (isspace(c)) break;
        --i;
    }
    size_t start = i;
    return name.substr(start, end - start + 1);
}

vector<string> lastNameLensort(const vector<string> &names) {
    vector<pair<string,string>> items;
    items.reserve(names.size());
    for (const auto &n : names) {
        string s = n; // copy (if n is empty that's fine)
        string ln = extract_last_name(s);
        items.emplace_back(move(s), move(ln));
    }
    stable_sort(items.begin(), items.end(), [](const auto &a, const auto &b){
        const string &la = a.second;
        const string &lb = b.second;
        if (la.size() != lb.size()) return la.size() < lb.size();
        string al = to_lower_copy(la), bl = to_lower_copy(lb);
        if (al != bl) return al < bl;
        string fa = to_lower_copy(a.first), fb = to_lower_copy(b.first);
        return fa < fb;
    });
    vector<string> out;
    out.reserve(items.size());
    for (auto &p : items) out.emplace_back(move(p.first));
    return out;
}

static void printVec(const vector<string>& v) {
    cout << "[";
    for (size_t i = 0; i < v.size(); ++i) {
        cout << (i ? ", " : "") << "\"" << v[i] << "\"";
    }
    cout << "]\n";
}

int main() {
    // Test case 1 (given example)
    vector<string> t1 = {
        "Jennifer Figueroa",
        "Heather Mcgee",
        "Amanda Schwartz",
        "Nicole Yoder",
        "Melissa Hoffman"
    };
    printVec(lastNameLensort(t1));

    // Test case 2 (all same last-name length -> alphabetical by last name)
    vector<string> t2 = {"Anna Zed", "Bob Kay", "Cara May", "Dan Ray"};
    printVec(lastNameLensort(t2));

    // Test case 3 (extra spaces and punctuation)
    vector<string> t3 = {"  John   Doe  ", "Alice   Smith", "Bob  O'Niel"};
    printVec(lastNameLensort(t3));

    // Test case 4 (same last name -> fall back to full name alphabetical)
    vector<string> t4 = {"Cindy Lee", "Anna Lee", "Brian Lee"};
    printVec(lastNameLensort(t4));

    // Test case 5 (edge cases)
    vector<string> t5 = {"", "Single", "Mary Ann", "  "};
    printVec(lastNameLensort(t5));

    return 0;
}