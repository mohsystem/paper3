#include <iostream>
#include <string>
using namespace std;

string check_mod(const string& username, const string& password) {
    if (username == "moderator" && password == "mod123") return "true";
    return "false";
}

int main() {
    cout << check_mod("moderator", "mod123") << endl;  // true
    cout << check_mod("moderator", "wrong") << endl;   // false
    cout << check_mod("user", "mod123") << endl;       // false
    cout << check_mod("moderator", "") << endl;        // false
    cout << check_mod("MODERATOR", "mod123") << endl;  // false
    return 0;
}