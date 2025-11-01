#include <iostream>
#include <unordered_map>
#include <string>
#include <stdexcept>
using namespace std;

string login(const string& username, const string& password) {
    try {
        static unordered_map<string, string> db = {
            {"alice", "password123"},
            {"bob", "qwerty"},
            {"charlie", "letmein"}
        };
        if (username == "error" || password == "error") {
            throw runtime_error("Forced error");
        }
        auto it = db.find(username);
        if (it != db.end() && it->second == password) {
            return "success";
        }
        return "cant_login";
    } catch (...) {
        return "internal_server_error";
    }
}

int main() {
    cout << login("alice", "password123") << endl; // success
    cout << login("alice", "wrong") << endl;       // cant_login
    cout << login("unknown", "pass") << endl;      // cant_login
    cout << login("error", "pass") << endl;        // internal_server_error
    cout << login("bob", "error") << endl;         // internal_server_error
    return 0;
}