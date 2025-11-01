
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
    User(string u, string e, string p) : username(u), email(e), password(p) {}
};

map<string, User> users;
string currentLoggedInUser = "";

bool login(string username, string password) {
    if (users.find(username) != users.end()) {
        User user = users[username];
        if (user.password == password) {
            currentLoggedInUser = username;
            return true;
        }
    }
    return false;
}

void logout() {
    currentLoggedInUser = "";
}

bool registerUser(string username, string email, string password) {
    if (users.find(username) == users.end()) {
        users[username] = User(username, email, password);
        return true;
    }
    return false;
}

string changeEmail(string oldEmail, string newEmail, string confirmPassword) {
    // Check if user is logged in
    if (currentLoggedInUser.empty()) {
        return "Error: User must be logged in to change email";
    }
    
    User& user = users[currentLoggedInUser];
    
    // Check if old email matches
    if (user.email != oldEmail) {
        return "Error: Old email does not match";
    }
    
    // Check if password is correct
    if (user.password != confirmPassword) {
        return "Error: Incorrect password";
    }
    
    // Check if new email is different from old email
    if (oldEmail == newEmail) {
        return "Error: New email must be different from old email";
    }
    
    // Change email
    user.email = newEmail;
    return "Success: Email changed successfully";
}

int main() {
    // Test Case 1: Change email successfully
    cout << "Test Case 1:" << endl;
    registerUser("user1", "old@example.com", "password123");
    login("user1", "password123");
    cout << changeEmail("old@example.com", "new@example.com", "password123") << endl;
    logout();
    cout << endl;
    
    // Test Case 2: Try to change email without logging in
    cout << "Test Case 2:" << endl;
    cout << changeEmail("old@example.com", "new2@example.com", "password123") << endl;
    cout << endl;
    
    // Test Case 3: Wrong old email
    cout << "Test Case 3:" << endl;
    registerUser("user2", "user2@example.com", "pass456");
    login("user2", "pass456");
    cout << changeEmail("wrong@example.com", "new@example.com", "pass456") << endl;
    logout();
    cout << endl;
    
    // Test Case 4: Wrong password
    cout << "Test Case 4:" << endl;
    registerUser("user3", "user3@example.com", "mypass");
    login("user3", "mypass");
    cout << changeEmail("user3@example.com", "newuser3@example.com", "wrongpass") << endl;
    logout();
    cout << endl;
    
    // Test Case 5: New email same as old email
    cout << "Test Case 5:" << endl;
    registerUser("user4", "user4@example.com", "secure123");
    login("user4", "secure123");
    cout << changeEmail("user4@example.com", "user4@example.com", "secure123") << endl;
    logout();
    
    return 0;
}
