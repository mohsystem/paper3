
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <openssl/sha.h>

using namespace std;

string hashPassword(const string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password.c_str(), password.length(), hash);
    
    stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

void registerUser(const string& username, const string& password) {
    string hashedPassword = hashPassword(password);
    cout << "Username: " << username << endl;
    cout << "Hashed Password: " << hashedPassword << endl;
}

int main() {
    // Test cases
    cout << "Test Case 1:" << endl;
    registerUser("john_doe", "password123");
    cout << endl;
    
    cout << "Test Case 2:" << endl;
    registerUser("alice", "securePass!@#");
    cout << endl;
    
    cout << "Test Case 3:" << endl;
    registerUser("bob_smith", "myP@ssw0rd");
    cout << endl;
    
    cout << "Test Case 4:" << endl;
    registerUser("user123", "12345678");
    cout << endl;
    
    cout << "Test Case 5:" << endl;
    registerUser("admin", "adminPass2024");
    cout << endl;
    
    return 0;
}
