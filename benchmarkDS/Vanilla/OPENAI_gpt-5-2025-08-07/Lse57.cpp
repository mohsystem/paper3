#include <bits/stdc++.h>
using namespace std;

class Auth {
    unordered_map<string, string> users;
    string currentEmail;
public:
    Auth(): currentEmail("") {}

    bool addUser(const string& email, const string& password) {
        if (email.empty()) return false;
        if (users.find(email) != users.end()) return false;
        users[email] = password;
        return true;
    }

    bool login(const string& email, const string& password) {
        auto it = users.find(email);
        if (it != users.end() && it->second == password) {
            currentEmail = email;
            return true;
        }
        return false;
    }

    void logout() {
        currentEmail.clear();
    }

    bool isLoggedIn() const {
        return !currentEmail.empty();
    }

    string getCurrentEmail() const {
        return currentEmail;
    }

    bool changeEmail(const string& oldEmail, const string& newEmail, const string& confirmPassword) {
        if (!isLoggedIn()) return false;
        auto it = users.find(oldEmail);
        if (it == users.end()) return false;                  // old email must exist
        if (oldEmail != currentEmail) return false;           // must match logged-in user
        if (it->second != confirmPassword) return false;      // confirm password must match
        if (users.find(newEmail) != users.end()) return false; // prevent duplicate email
        string pw = it->second;
        users.erase(it);
        users[newEmail] = pw;
        currentEmail = newEmail;
        return true;
    }
};

static Auth seed() {
    Auth a;
    a.addUser("alice@example.com", "alice123");
    a.addUser("bob@example.com", "bobpwd");
    a.addUser("charlie@example.com", "charlie!");
    return a;
}

int main() {
    // Test 1: Successful login and change
    Auth t1 = seed();
    bool t1Login = t1.login("alice@example.com", "alice123");
    bool t1Change = t1.changeEmail("alice@example.com", "alice.new@example.com", "alice123");
    cout << "Test1 login=" << (t1Login ? "true" : "false")
         << " change=" << (t1Change ? "true" : "false")
         << " current=" << t1.getCurrentEmail() << "\n";

    // Test 2: Attempt change without login
    Auth t2 = seed();
    bool t2Change = t2.changeEmail("bob@example.com", "bob.new@example.com", "bobpwd");
    cout << "Test2 changeWithoutLogin=" << (t2Change ? "true" : "false") << "\n";

    // Test 3: Wrong confirm password
    Auth t3 = seed();
    bool t3Login = t3.login("bob@example.com", "bobpwd");
    bool t3Change = t3.changeEmail("bob@example.com", "bob.new@example.com", "wrong");
    cout << "Test3 login=" << (t3Login ? "true" : "false")
         << " changeWithWrongPassword=" << (t3Change ? "true" : "false") << "\n";

    // Test 4: Non-existing old email while logged in
    Auth t4 = seed();
    bool t4Login = t4.login("charlie@example.com", "charlie!");
    bool t4Change = t4.changeEmail("ghost@example.com", "ghost2@example.com", "charlie!");
    cout << "Test4 login=" << (t4Login ? "true" : "false")
         << " changeWithNonExistingOldEmail=" << (t4Change ? "true" : "false") << "\n";

    // Test 5: New email already in use
    Auth t5 = seed();
    bool t5Login = t5.login("alice@example.com", "alice123");
    bool t5Change = t5.changeEmail("alice@example.com", "bob@example.com", "alice123");
    cout << "Test5 login=" << (t5Login ? "true" : "false")
         << " changeToExistingEmail=" << (t5Change ? "true" : "false") << "\n";

    return 0;
}