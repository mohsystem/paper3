#include <iostream>
#include <unordered_map>
#include <string>
using namespace std;

bool authenticate(const unordered_map<string, string>& users, const string& username, const string& password) {
    auto it = users.find(username);
    if (it == users.end()) return false;
    return it->second == password;
}

int main() {
    unordered_map<string, string> users{
        {"alice", "pass123"},
        {"bob", "qwerty"},
        {"charlie", "letmein"}
    };

    cout << boolalpha;
    cout << "Test 1 (alice, correct): " << authenticate(users, "alice", "pass123") << "\n";
    cout << "Test 2 (bob, wrong): " << authenticate(users, "bob", "wrong") << "\n";
    cout << "Test 3 (unknown user): " << authenticate(users, "eve", "anything") << "\n";
    cout << "Test 4 (charlie, empty password): " << authenticate(users, "charlie", "") << "\n";
    cout << "Test 5 (case sensitivity): " << authenticate(users, "Alice", "pass123") << "\n";
    return 0;
}