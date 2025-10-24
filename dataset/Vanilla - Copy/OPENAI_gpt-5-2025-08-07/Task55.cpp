#include <bits/stdc++.h>
using namespace std;

bool isValidEmail(const string& email) {
    if (email.empty() || email.size() > 254) return false;

    size_t atFirst = email.find('@');
    size_t atLast = email.rfind('@');
    if (atFirst == string::npos || atFirst != atLast || atFirst == 0 || atFirst == email.size() - 1) return false;

    string local = email.substr(0, atFirst);
    string domain = email.substr(atFirst + 1);

    if (local.empty() || local.size() > 64) return false;

    if (local.front() == '.' || local.back() == '.') return false;
    char prev = '\0';
    for (char c : local) {
        bool allowed = isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
        if (!allowed) return false;
        if (c == '.' && prev == '.') return false;
        prev = c;
    }

    if (domain.empty()) return false;
    vector<string> labels;
    {
        string cur;
        for (char c : domain) {
            if (c == '.') {
                labels.push_back(cur);
                cur.clear();
            } else {
                cur.push_back(c);
            }
        }
        labels.push_back(cur);
    }
    if (labels.size() < 2) return false;

    for (auto &label : labels) {
        if (label.empty() || label.size() > 63) return false;
        if (!isalnum(static_cast<unsigned char>(label.front())) || !isalnum(static_cast<unsigned char>(label.back())))
            return false;
        for (char c : label) {
            if (!(isalnum(static_cast<unsigned char>(c)) || c == '-')) return false;
        }
    }
    const string& tld = labels.back();
    if (tld.size() < 2) return false;
    for (char c : tld) {
        if (!isalpha(static_cast<unsigned char>(c))) return false;
    }

    return true;
}

int main() {
    vector<string> tests = {
        "user@example.com",
        "first.last+tag@sub.domain.co",
        ".user@domain.com",
        "user@-domain.com",
        "user@domain"
    };
    for (const auto& e : tests) {
        cout << e << " => " << (isValidEmail(e) ? "true" : "false") << "\n";
    }
    return 0;
}