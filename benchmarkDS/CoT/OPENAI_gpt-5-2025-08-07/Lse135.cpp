#include <iostream>
#include <string>
using namespace std;

string check_mod(const string& username, const string& password) {
    return (username == "admin" && password == "secret") ? "true" : "false";
}

int main() {
    pair<string, string> tests[5] = {
        {"admin", "secret"},
        {"admin", "wrong"},
        {"user", "secret"},
        {"admin", ""},
        {"Admin", "secret"}
    };
    for (auto& t : tests) {
        cout << check_mod(t.first, t.second) << endl;
    }
    return 0;
}