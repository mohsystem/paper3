#include <bits/stdc++.h>
using namespace std;

static unordered_set<string> users_db = {"alice@example.com", "bob@example.com", "carol@example.com"};
static unordered_set<string> mailing_list = users_db;

static string normalize(const string& email) {
    string s = email;
    // trim
    auto l = s.begin();
    while (l != s.end() && isspace(static_cast<unsigned char>(*l))) ++l;
    auto r = s.end();
    do { if (r == s.begin()) break; --r; } while (isspace(static_cast<unsigned char>(*r)));
    if (l == s.end()) s = "";
    else s = string(l, r + 1);
    // tolower
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

// Simulates handling a GET /unsubscribe?email=<email>
string unsubscribe_get(const string& email) {
    if (email.find_first_not_of(" \t\n\r\f\v") == string::npos) {
        return "Invalid email parameter.";
    }
    string norm = normalize(email);
    if (users_db.find(norm) != users_db.end()) {
        users_db.erase(norm);         // delete user from the "database"
        mailing_list.erase(norm);     // unsubscribe from mailing list
        return norm + " has been unsubscribed.";
    } else {
        return norm + " is not subscribed.";
    }
}

int main() {
    vector<string> tests = {
        "alice@example.com",   // subscribed -> unsubscribe
        "dave@example.com",    // not subscribed
        "bob@example.com",     // subscribed -> unsubscribe
        "bob@example.com",     // already unsubscribed
        "Carol@Example.com"    // case-insensitive match -> unsubscribe
    };
    for (auto& t : tests) {
        cout << unsubscribe_get(t) << "\n";
    }
    return 0;
}