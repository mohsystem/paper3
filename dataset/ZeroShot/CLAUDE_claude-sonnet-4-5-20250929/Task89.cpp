
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

using namespace std;

bool comparePasswords(const string& storedPassword, const string& enteredPassword) {
    if (storedPassword.length() != enteredPassword.length()) {
        return false;
    }
    
    for (size_t i = 0; i < storedPassword.length(); i++) {
        if (storedPassword[i] != enteredPassword[i]) {
            return false;
        }
    }
    
    return true;
}

string readPasswordFromFile(const string& filename) {
    ifstream file(filename);
    string password;
    
    if (file.is_open()) {
        getline(file, password);
        file.close();
        return password;
    } else {
        cerr << "Error reading password file" << endl;
        return "";
    }
}

bool verifyPassword(const string& filename, const string& enteredPassword) {
    string storedPassword = readPasswordFromFile(filename);
    return comparePasswords(storedPassword, enteredPassword);
}

int main() {
    // Create a test password file
    string testFile = "password.txt";
    ofstream outFile(testFile);
    if (outFile.is_open()) {
        outFile << "SecurePass123" << endl;
        outFile.close();
    }
    
    // Test cases
    cout << "Test Case 1 - Correct password:" << endl;
    bool result1 = verifyPassword(testFile, "SecurePass123");
    cout << "Result: " << (result1 ? "Access Granted" : "Access Denied") << endl;
    
    cout << "\\nTest Case 2 - Incorrect password:" << endl;
    bool result2 = verifyPassword(testFile, "WrongPass456");
    cout << "Result: " << (result2 ? "Access Granted" : "Access Denied") << endl;
    
    cout << "\\nTest Case 3 - Empty password:" << endl;
    bool result3 = verifyPassword(testFile, "");
    cout << "Result: " << (result3 ? "Access Granted" : "Access Denied") << endl;
    
    cout << "\\nTest Case 4 - Case sensitive check:" << endl;
    bool result4 = verifyPassword(testFile, "securepass123");
    cout << "Result: " << (result4 ? "Access Granted" : "Access Denied") << endl;
    
    cout << "\\nTest Case 5 - Partial match:" << endl;
    bool result5 = verifyPassword(testFile, "SecurePass");
    cout << "Result: " << (result5 ? "Access Granted" : "Access Denied") << endl;
    
    // Clean up test file
    remove(testFile.c_str());
    
    return 0;
}
