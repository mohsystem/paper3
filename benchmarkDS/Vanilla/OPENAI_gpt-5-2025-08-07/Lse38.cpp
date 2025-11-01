#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Returns "dashboard" if credentials are correct, otherwise "login"
string login(const string& username, const string& password) {
    static const unordered_map<string, string> db = {
        {"alice", "password123"},
        {"bob", "qwerty"},
        {"charlie", "charlie!"},
        {"dana", "D@n@2025"},
        {"eve", "12345"}
    };
    auto it = db.find(username);
    if (it != db.end() && it->second == password) return "dashboard";
    return "login";
}

int main() {
    // 5 test cases
    vector<pair<string, string>> tests = {
        {"alice", "password123"}, // correct
        {"alice", "wrong"},       // wrong password
        {"unknown", "whatever"},  // unknown user
        {"Bob", "qwerty"},        // case sensitivity
        {"", ""}                  // empty credentials
    };

    for (const auto& t : tests) {
        cout << "login(\"" << t.first << "\", \"" << t.second << "\") => " << login(t.first, t.second) << "\n";
    }
    return 0;
}