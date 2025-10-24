#include <bits/stdc++.h>
using namespace std;

// Step 1-5 combined into secure implementation

static bool hasConsecutiveSequenceDigits(const string& s) {
    int run = 1;
    char prev = 0;
    for (char ch : s) {
        if (!isdigit(static_cast<unsigned char>(ch))) {
            run = 1;
            prev = 0;
            continue;
        }
        if (prev != 0) {
            if (ch == prev + 1 || ch == prev - 1) {
                run++;
                if (run >= 4) return true;
            } else {
                run = 1;
            }
        }
        prev = ch;
    }
    return false;
}

static bool hasConsecutiveSequenceLetters(const string& s) {
    int run = 1;
    char prev = 0;
    for (char raw : s) {
        if (!isalpha(static_cast<unsigned char>(raw))) {
            run = 1;
            prev = 0;
            continue;
        }
        char ch = static_cast<char>(tolower(static_cast<unsigned char>(raw)));
        if (prev != 0) {
            if (ch == prev + 1 || ch == prev - 1) {
                run++;
                if (run >= 4) return true;
            } else {
                run = 1;
            }
        }
        prev = ch;
    }
    return false;
}

bool isValidAdminPassword(const string& pwd) {
    if (pwd.empty()) return false;
    if (pwd.size() < 12 || pwd.size() > 128) return false;

    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;

    int repeatCount = 1;
    for (size_t i = 0; i < pwd.size(); ++i) {
        unsigned char ch = static_cast<unsigned char>(pwd[i]);

        if (isspace(ch)) return false;

        if (isupper(ch)) hasUpper = true;
        else if (islower(ch)) hasLower = true;
        else if (isdigit(ch)) hasDigit = true;

        if (!isalnum(ch) && !isspace(ch)) hasSpecial = true;

        if (i > 0) {
            if (pwd[i] == pwd[i - 1]) {
                repeatCount++;
                if (repeatCount >= 3) return false;
            } else {
                repeatCount = 1;
            }
        }
    }

    if (!(hasUpper && hasLower && hasDigit && hasSpecial)) return false;

    string lower = pwd;
    transform(lower.begin(), lower.end(), lower.begin(),
              [](unsigned char c){ return static_cast<char>(tolower(c)); });
    vector<string> banned = {"password", "admin", "qwerty", "letmein", "123456"};
    for (const auto& b : banned) {
        if (lower.find(b) != string::npos) return false;
    }

    if (hasConsecutiveSequenceDigits(pwd) || hasConsecutiveSequenceLetters(pwd)) return false;

    return true;
}

int main() {
    vector<string> tests = {
        "password123!",
        "Admin123!",
        "ValidStrong#2024Xx",
        "NoSpecialChar1234",
        "AAAaaa111!!!"
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << ": " << (isValidAdminPassword(tests[i]) ? "1" : "0") << "\n";
    }
    return 0;
}