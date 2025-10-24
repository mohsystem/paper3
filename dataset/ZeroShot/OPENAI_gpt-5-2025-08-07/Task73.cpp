#include <bits/stdc++.h>
using namespace std;

static const int MIN_LENGTH = 12;
static const int MAX_LENGTH = 128;

static const unordered_set<string> COMMON_PASSWORDS = {
    "password","123456","qwerty","letmein","welcome","admin","administrator","passw0rd",
    "iloveyou","monkey","dragon","football","abc123","123456789","12345678","111111",
    "123123","login","princess","admin123","adminadmin","root","root123"
};

static inline bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z');
}

static bool hasTripleRepeat(const string& s) {
    int cnt = 1;
    for (size_t i = 1; i < s.size(); ++i) {
        if (s[i] == s[i-1]) {
            cnt++;
            if (cnt >= 3) return true;
        } else cnt = 1;
    }
    return false;
}

static bool hasSequentialPattern(const string& s) {
    string l = s;
    for (char& c : l) c = (char)tolower((unsigned char)c);
    int n = (int)l.size();
    for (int i = 0; i <= n - 4; ++i) {
        // letters
        if (isAlpha(l[i]) && isAlpha(l[i+1]) && isAlpha(l[i+2]) && isAlpha(l[i+3])) {
            bool asc = true, desc = true;
            for (int k = i; k < i + 3; ++k) {
                if (l[k+1] != (char)(l[k] + 1)) asc = false;
                if (l[k+1] != (char)(l[k] - 1)) desc = false;
            }
            if (asc || desc) return true;
        }
        // digits
        if (isdigit((unsigned char)l[i]) && isdigit((unsigned char)l[i+1]) &&
            isdigit((unsigned char)l[i+2]) && isdigit((unsigned char)l[i+3])) {
            bool asc = true, desc = true;
            for (int k = i; k < i + 3; ++k) {
                if (l[k+1] != (char)(l[k] + 1)) asc = false;
                if (l[k+1] != (char)(l[k] - 1)) desc = false;
            }
            if (asc || desc) return true;
        }
    }
    return false;
}

bool isValidAdminPassword(const string& password) {
    if (password.empty()) return false;
    int n = (int)password.size();
    if (n < MIN_LENGTH || n > MAX_LENGTH) return false;

    string lower = password;
    for (char& c : lower) c = (char)tolower((unsigned char)c);

    if (COMMON_PASSWORDS.find(lower) != COMMON_PASSWORDS.end()) return false;
    if (lower.find("admin") != string::npos || lower.find("administrator") != string::npos || lower.find("root") != string::npos)
        return false;

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (char c : password) {
        if (isspace((unsigned char)c)) return false;
        if (isupper((unsigned char)c)) hasUpper = true;
        else if (islower((unsigned char)c)) hasLower = true;
        else if (isdigit((unsigned char)c)) hasDigit = true;
        else hasSpecial = true;
    }
    if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return false;

    if (hasTripleRepeat(password)) return false;
    if (hasSequentialPattern(password)) return false;

    return true;
}

int main() {
    vector<string> tests = {
        "password",          // false
        "Admin123!",         // false
        "StrongPassw0rd!",   // true
        "Adm!nStrong#2024",  // true
        "Abcd1234!xyZ"       // false
    };
    for (const auto& t : tests) {
        cout << (isValidAdminPassword(t) ? "true" : "false") << "\n";
    }
    return 0;
}