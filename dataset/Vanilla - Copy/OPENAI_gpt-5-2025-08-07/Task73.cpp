#include <bits/stdc++.h>
using namespace std;

bool isValidAdminPassword(const string& pwd) {
    if (pwd.size() < 12) return false;

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;

    char prev1 = 0, prev2 = 0;
    for (size_t i = 0; i < pwd.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(pwd[i]);
        if (isspace(ch)) return false;

        if (isupper(ch)) hasUpper = true;
        else if (islower(ch)) hasLower = true;
        else if (isdigit(ch)) hasDigit = true;
        else hasSpecial = true;

        if (i >= 2) {
            if (pwd[i] == prev1 && pwd[i] == prev2) return false; // 3 identical in a row
        }
        prev2 = prev1;
        prev1 = pwd[i];
    }

    if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return false;

    string lowerPwd = pwd;
    for (char& c : lowerPwd) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));

    vector<string> badEquals = {"password", "admin", "admin123", "123456", "qwerty", "letmein", "welcome", "passw0rd", "iloveyou"};
    for (const auto& s : badEquals) {
        if (lowerPwd == s) return false;
    }
    vector<string> badContains = {"admin", "password", "qwerty", "12345"};
    for (const auto& s : badContains) {
        if (lowerPwd.find(s) != string::npos) return false;
    }

    return true;
}

int main() {
    vector<string> tests = {
        "Admin@123",
        "Str0ng!AdminPwd",
        "CorrectHorse9!",
        "aaaaaaaaaaaa",
        "N0_Spaces-Allowed?"
    };
    for (const auto& t : tests) {
        cout << t << " -> " << (isValidAdminPassword(t) ? "true" : "false") << "\n";
    }
    return 0;
}