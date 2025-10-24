#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
using namespace std;

string ltrim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) start++;
    return s.substr(start);
}

string rtrim(const string& s) {
    if (s.empty()) return s;
    size_t end = s.size();
    while (end > 0 && isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(0, end);
}

string trim(const string& s) {
    return rtrim(ltrim(s));
}

string tolower_str(string s) {
    transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(tolower(c)); });
    return s;
}

string redirect(const string& input) {
    string key = tolower_str(trim(input));
    if (key.empty()) return "/home";

    if (key.rfind("profile", 0) == 0) return "/user/profile";

    if (key == "home") return "/home";
    if (key == "login") return "/auth/login";
    if (key == "logout") return "/auth/logout";
    if (key == "admin") return "/admin/dashboard";
    if (key == "help") return "/support/help";
    if (key == "settings") return "/user/settings";

    return "/error/404";
}

int main() {
    string tests[5] = {
        "home",
        "LOGIN",
        "Admin ",
        "profile?id=42",
        "unknown"
    };

    for (const auto& t : tests) {
        cout << t << " -> " << redirect(t) << "\n";
    }
    return 0;
}