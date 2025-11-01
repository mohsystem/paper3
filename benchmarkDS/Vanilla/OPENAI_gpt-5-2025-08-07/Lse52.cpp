#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

string index_page() {
    return "<html><body><a href='/login'>Login</a></body></html>";
}

string login_page() {
    return "<html><body><form method='post' action='/do_login'>"
           "Username: <input type='text' name='username'/> "
           "Password: <input type='password' name='password'/> "
           "<button type='submit'>Login</button>"
           "</form></body></html>";
}

bool credentials_valid(const string& u, const string& p) {
    return (u == "admin" && p == "password") || (u == "alice" && p == "wonderland");
}

string do_login(const string& username, const string& password, unordered_map<string, string>& session) {
    if (credentials_valid(username, password)) {
        session["username"] = username;
        return "REDIRECT:/secret";
    } else {
        return "REDIRECT:/login?error=1";
    }
}

string secret_page(const unordered_map<string, string>& session) {
    if (session.find("username") != session.end()) {
        return "Secret settings";
    }
    return "REDIRECT:/login";
}

int main() {
    // Test 1: index page
    cout << "TEST1 index: " << index_page() << "\n";
    // Test 2: login page
    cout << "TEST2 login page: " << login_page() << "\n";
    // Test 3: failed login attempt
    unordered_map<string, string> session1;
    cout << "TEST3 do_login fail: " << do_login("admin", "wrong", session1) << "\n";
    cout << "TEST3 secret after fail: " << secret_page(session1) << "\n";
    // Test 4: successful login (admin)
    unordered_map<string, string> session2;
    cout << "TEST4 do_login success: " << do_login("admin", "password", session2) << "\n";
    cout << "TEST4 secret after success: " << secret_page(session2) << "\n";
    // Test 5: successful login (alice)
    unordered_map<string, string> session3;
    cout << "TEST5 do_login success: " << do_login("alice", "wonderland", session3) << "\n";
    cout << "TEST5 secret after success: " << secret_page(session3) << "\n";
    return 0;
}