#include <iostream>
#include <string>
#include <vector>
using namespace std;

string check_mod(const string& username, const string& password) {
    if (username == "admin" && password == "secret") return "true";
    return "false";
}

int main() {
    vector<pair<string, string>> tests = {
        {"admin", "secret"},
        {"Admin", "secret"},
        {"admin", "Secret"},
        {"user", "secret"},
        {"admin", "secret123"}
    };
    for (size_t i = 0; i < tests.size(); ++i) {
        cout << "Test " << (i + 1) << ": " << check_mod(tests[i].first, tests[i].second) << "\n";
    }
    return 0;
}