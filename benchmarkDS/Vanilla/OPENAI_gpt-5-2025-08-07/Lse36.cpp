#include <bits/stdc++.h>
using namespace std;

static unordered_map<string, string> DB = {
    {"alice", "wonderland"},
    {"bob", "builder"},
    {"admin", "admin123"}
};

string login(const string& username, const string& password) {
    auto it = DB.find(username);
    if (it == DB.end()) return "return";
    if (it->second == password) return "redirect:/home";
    return "return";
}

int main() {
    // 5 test cases
    cout << login("alice", "wonderland") << endl; // success -> redirect
    cout << login("alice", "wrong") << endl;      // wrong password -> return
    cout << login("bob", "builder") << endl;      // success -> redirect
    cout << login("charlie", "pass") << endl;     // user not found -> return
    cout << login("", "") << endl;                // empty credentials -> return
    return 0;
}