
#include <iostream>
#include <string>
#include <map>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using namespace std;

class Lse39 {
private:
    static map<string, string> userDatabase;
    
    // Hash password using SHA-256
    static string hashPassword(const string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    // Initialize user database
    static void initialize() {
        userDatabase["admin"] = hashPassword("admin123");
        userDatabase["user1"] = hashPassword("password1");
        userDatabase["user2"] = hashPassword("password2");
        userDatabase["testuser"] = hashPassword("test123");
        userDatabase["john"] = hashPassword("john456");
    }
    
    // Secure login function
    static string login(const string& username, const string& password) {
        // Input validation
        if (username.empty() || password.empty()) {
            return "Error: Username or password cannot be empty";
        }
        
        // Check if user exists and password matches
        auto it = userDatabase.find(username);
        if (it != userDatabase.end() && it->second == hashPassword(password)) {
            return "Success: Redirecting to session page for user: " + username;
        } else {
            return "Error: Incorrect username or password";
        }
    }
};

map<string, string> Lse39::userDatabase;

int main() {
    Lse39::initialize();
    
    cout << "=== Secure Login System Test Cases ===" << endl << endl;
    
    // Test Case 1: Valid credentials
    cout << "Test Case 1: Valid login (admin/admin123)" << endl;
    cout << Lse39::login("admin", "admin123") << endl << endl;
    
    // Test Case 2: Invalid password
    cout << "Test Case 2: Invalid password (admin/wrongpass)" << endl;
    cout << Lse39::login("admin", "wrongpass") << endl << endl;
    
    // Test Case 3: Non-existent user
    cout << "Test Case 3: Non-existent user (hacker/hack123)" << endl;
    cout << Lse39::login("hacker", "hack123") << endl << endl;
    
    // Test Case 4: Empty credentials
    cout << "Test Case 4: Empty username" << endl;
    cout << Lse39::login("", "password") << endl << endl;
    
    // Test Case 5: Valid user2 login
    cout << "Test Case 5: Valid login (user2/password2)" << endl;
    cout << Lse39::login("user2", "password2") << endl;
    
    return 0;
}
