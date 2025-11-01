
#include <iostream>
#include <string>
#include <map>
using namespace std;

class UserInfo {
public:
    string username;
    string email;
    string ssn;
    
    UserInfo() {}
    
    UserInfo(string u, string e, string s) : username(u), email(e), ssn(s) {}
    
    string getLast4SSN() {
        if (ssn.length() >= 4) {
            return ssn.substr(ssn.length() - 4);
        }
        return "****";
    }
};

map<string, UserInfo> userDatabase;

string getInfoPage(string userId) {
    if (userDatabase.find(userId) == userDatabase.end()) {
        return "User not found";
    }
    
    UserInfo user = userDatabase[userId];
    string response = "";
    response += "User Information\\n";
    response += "================\\n";
    response += "Username: " + user.username + "\\n";
    response += "Email: " + user.email + "\\n";
    response += "SSN (Last 4): " + user.getLast4SSN() + "\\n";
    
    return response;
}

int main() {
    // Initialize test data
    userDatabase["user1"] = UserInfo("john_doe", "john@example.com", "123456789");
    userDatabase["user2"] = UserInfo("jane_smith", "jane@example.com", "987654321");
    userDatabase["user3"] = UserInfo("bob_wilson", "bob@example.com", "555443333");
    userDatabase["user4"] = UserInfo("alice_brown", "alice@example.com", "111222333");
    userDatabase["user5"] = UserInfo("charlie_davis", "charlie@example.com", "999888777");
    
    // Test cases
    cout << "Test Case 1:" << endl;
    cout << getInfoPage("user1") << endl;
    
    cout << "Test Case 2:" << endl;
    cout << getInfoPage("user2") << endl;
    
    cout << "Test Case 3:" << endl;
    cout << getInfoPage("user3") << endl;
    
    cout << "Test Case 4:" << endl;
    cout << getInfoPage("user4") << endl;
    
    cout << "Test Case 5:" << endl;
    cout << getInfoPage("user5") << endl;
    
    return 0;
}
