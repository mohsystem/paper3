#include <iostream>
#include <string>

struct User {
    std::string username;
    std::string email;
    std::string password;
    bool loggedIn{false};
};

bool login(User& user, const std::string& password) {
    if (user.password == password) {
        user.loggedIn = true;
        return true;
    }
    return false;
}

bool changeEmail(User& user, const std::string& oldEmail, const std::string& password, const std::string& newEmail) {
    if (!user.loggedIn) return false;
    if (user.email != oldEmail) return false;
    if (user.password != password) return false;
    user.email = newEmail;
    return true;
}

int main() {
    // Test 1: Not logged in
    User u1{"bob", "bob@example.com", "bobpass", false};
    bool t1 = changeEmail(u1, "bob@example.com", "bobpass", "newbob@example.com");
    std::cout << "Test1 (not logged in, expect false): " << (t1 ? "true" : "false") << " | email=" << u1.email << "\n";

    // Test 2: Correct login + change
    User u2{"alice", "alice@example.com", "pass123", false};
    bool login2 = login(u2, "pass123");
    bool t2 = changeEmail(u2, "alice@example.com", "pass123", "alice_new@example.com");
    std::cout << "Test2 (logged in, correct creds, expect true): " << ((login2 && t2) ? "true" : "false") << " | email=" << u2.email << "\n";

    // Test 3: Wrong old email
    User u3{"carol", "carol@example.com", "cpass", false};
    login(u3, "cpass");
    bool t3 = changeEmail(u3, "wrong@example.com", "cpass", "carol_new@example.com");
    std::cout << "Test3 (wrong old email, expect false): " << (t3 ? "true" : "false") << " | email=" << u3.email << "\n";

    // Test 4: Wrong password
    User u4{"dan", "dan@example.com", "dpass", false};
    login(u4, "dpass");
    bool t4 = changeEmail(u4, "dan@example.com", "wrongpass", "dan_new@example.com");
    std::cout << "Test4 (wrong password, expect false): " << (t4 ? "true" : "false") << " | email=" << u4.email << "\n";

    // Test 5: After change, using original old email should fail
    User u5{"eve", "eve@example.com", "epass", false};
    login(u5, "epass");
    bool firstChange = changeEmail(u5, "eve@example.com", "epass", "eve_new@example.com");
    bool secondChange = changeEmail(u5, "eve@example.com", "epass", "eve_new2@example.com");
    std::cout << "Test5 (first true then second false): first=" << (firstChange ? "true" : "false")
              << ", second=" << (secondChange ? "true" : "false") << " | email=" << u5.email << "\n";
    return 0;
}