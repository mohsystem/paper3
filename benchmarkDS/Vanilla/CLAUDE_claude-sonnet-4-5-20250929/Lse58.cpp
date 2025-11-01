
#include <iostream>
#include <string>
#include <map>
using namespace std;

class User {
public:
    string username;
    string email;
    string password;
    
    User() {}
    User(string username, string email, string password) {
        this->username = username;
        this->email = email;
        this->password = password;
    }
};

map<string, User> users;
User* loggedInUser = nullptr;

bool registerUser(string username, string email, string password) {
    if (users.find(username) != users.end()) {
        return false;
    }
    users[username] = User(username, email, password);
    return true;
}

bool login(string username, string password) {
    if (users.find(username) != users.end() && users[username].password == password) {
        loggedInUser = &users[username];
        return true;
    }
    return false;
}

void logout() {
    loggedInUser = nullptr;
}

bool changeEmail(string oldEmail, string newEmail, string password) {
    if (loggedInUser == nullptr) {
        return false;
    }
    
    if (loggedInUser->email == oldEmail && loggedInUser->password == password) {
        loggedInUser->email = newEmail;
        return true;
    }
    return false;
}

string getCurrentEmail() {
    if (loggedInUser != nullptr) {
        return loggedInUser->email;
    }
    return "";
}

int main() {
    // Test Case 1: Register and login, then change email successfully
    cout << "Test Case 1:" << endl;
    registerUser("user1", "user1@example.com", "password123");
    login("user1", "password123");
    bool result1 = changeEmail("user1@example.com", "newemail1@example.com", "password123");
    cout << "Change email result: " << result1 << ", New email: " << getCurrentEmail() << endl;
    logout();
    
    // Test Case 2: Try to change email without logging in
    cout << "\\nTest Case 2:" << endl;
    bool result2 = changeEmail("user1@example.com", "newemail2@example.com", "password123");
    cout << "Change email without login: " << result2 << endl;
    
    // Test Case 3: Login but provide wrong old email
    cout << "\\nTest Case 3:" << endl;
    login("user1", "password123");
    bool result3 = changeEmail("wrong@example.com", "newemail3@example.com", "password123");
    cout << "Change email with wrong old email: " << result3 << ", Current email: " << getCurrentEmail() << endl;
    logout();
    
    // Test Case 4: Login but provide wrong password
    cout << "\\nTest Case 4:" << endl;
    login("user1", "password123");
    bool result4 = changeEmail("newemail1@example.com", "newemail4@example.com", "wrongpassword");
    cout << "Change email with wrong password: " << result4 << ", Current email: " << getCurrentEmail() << endl;
    logout();
    
    // Test Case 5: Successful email change with correct credentials
    cout << "\\nTest Case 5:" << endl;
    registerUser("user2", "user2@example.com", "pass456");
    login("user2", "pass456");
    bool result5 = changeEmail("user2@example.com", "user2new@example.com", "pass456");
    cout << "Change email result: " << result5 << ", New email: " << getCurrentEmail() << endl;
    logout();
    
    return 0;
}
