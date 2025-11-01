#include <bits/stdc++.h>
using namespace std;

// GET /login
string login(const string& query) {
    return string("GET /login -> provide credentials via POST to /do_login. Query=") + query;
}

// POST /do_login
string do_login(const string& username, const string& password) {
    if (username == "admin" && password == "secret") return "admin|admin";
    if (username == "user" && password == "pass") return "user|user";
    return "ERROR: Invalid credentials";
}

// GET /
string index(const string& sessionToken) {
    if (sessionToken.empty() || sessionToken.rfind("ERROR", 0) == 0) return "Welcome, Guest";
    auto pos = sessionToken.find('|');
    string user = pos == string::npos ? sessionToken : sessionToken.substr(0, pos);
    return "Welcome, " + user;
}

// GET /user
string user_page(const string& sessionToken) {
    if (sessionToken.empty() || sessionToken.rfind("ERROR", 0) == 0) return "401 Unauthorized";
    auto pos = sessionToken.find('|');
    if (pos == string::npos) return "401 Unauthorized";
    string user = sessionToken.substr(0, pos);
    string role = sessionToken.substr(pos + 1);
    if (role == "user" || role == "admin") return "User Page for " + user;
    return "401 Unauthorized";
}

// GET /admin
string admin_page(const string& sessionToken) {
    if (sessionToken.empty() || sessionToken.rfind("ERROR", 0) == 0) return "401 Unauthorized";
    auto pos = sessionToken.find('|');
    if (pos == string::npos) return "401 Unauthorized";
    string user = sessionToken.substr(0, pos);
    string role = sessionToken.substr(pos + 1);
    if (role == "admin") return "Admin Page for " + user;
    return "401 Unauthorized";
}

int main() {
    // 5 test cases
    cout << "Test 1: " << login("next=/home") << "\n";
    string tokenUser = do_login("user", "pass");
    cout << "Test 2: " << index(tokenUser) << "\n";
    cout << "Test 3: " << user_page(tokenUser) << "\n";
    cout << "Test 4: " << admin_page(tokenUser) << "\n";
    string tokenAdmin = do_login("admin", "secret");
    cout << "Test 5: " << admin_page(tokenAdmin) << "\n";
    return 0;
}