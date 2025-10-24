
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

using namespace std;

class Task122 {
private:
    struct UserCredentials {
        string username;
        string hashedPassword;
        string salt;
    };
    
    map<string, UserCredentials> userDatabase;
    
    string generateSalt() {
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis(0, 255);
        
        stringstream ss;
        for (int i = 0; i < 16; i++) {
            ss << hex << setw(2) << setfill('0') << dis(gen);
        }
        return ss.str();
    }
    
    string hashPassword(const string& password, const string& salt) {
        string combined = salt + password;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << hex << setw(2) << setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    string signup(const string& username, const string& password) {
        if (username.empty() || username.find_first_not_of(" \\t\\n\\r") == string::npos) {
            return "Error: Username cannot be empty";
        }
        if (password.empty() || password.length() < 6) {
            return "Error: Password must be at least 6 characters";
        }
        if (userDatabase.find(username) != userDatabase.end()) {
            return "Error: Username already exists";
        }
        
        string salt = generateSalt();
        string hashedPassword = hashPassword(password, salt);
        
        UserCredentials creds;
        creds.username = username;
        creds.hashedPassword = hashedPassword;
        creds.salt = salt;
        
        userDatabase[username] = creds;
        return "Success: User " + username + " registered successfully";
    }
    
    bool verifyPassword(const string& username, const string& password) {
        if (userDatabase.find(username) == userDatabase.end()) {
            return false;
        }
        UserCredentials user = userDatabase[username];
        string hashedInput = hashPassword(password, user.salt);
        return hashedInput == user.hashedPassword;
    }
};

int main() {
    Task122 system;
    
    cout << "Test Case 1: Valid signup" << endl;
    cout << system.signup("john_doe", "password123") << endl;
    
    cout << "\\nTest Case 2: Duplicate username" << endl;
    cout << system.signup("john_doe", "newpassword456") << endl;
    
    cout << "\\nTest Case 3: Weak password" << endl;
    cout << system.signup("jane_doe", "123") << endl;
    
    cout << "\\nTest Case 4: Empty username" << endl;
    cout << system.signup("", "password123") << endl;
    
    cout << "\\nTest Case 5: Multiple valid signups and verification" << endl;
    cout << system.signup("alice", "securePass789") << endl;
    cout << system.signup("bob", "myPassword!") << endl;
    cout << "Verify alice password: " << (system.verifyPassword("alice", "securePass789") ? "true" : "false") << endl;
    cout << "Verify bob wrong password: " << (system.verifyPassword("bob", "wrongPass") ? "true" : "false") << endl;
    
    return 0;
}
