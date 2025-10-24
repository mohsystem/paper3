
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
    }
    
    return "";
}

bool verifyPassword(const string& filename, const string& enteredPassword) {
    string storedPassword = readPasswordFromFile(filename);
    return comparePasswords(storedPassword, enteredPassword);
}

int main() {
    // Create test file with password
    ofstream outFile("password.txt");
    if (outFile.is_open()) {
        outFile << "SecurePass123";
        outFile.close();
    }
    
    // Test Case 1: Correct password
    cout << "Test 1 - Correct password:" << endl;
    bool result1 = verifyPassword("password.txt", "SecurePass123");
    cout << "Result: " << (result1 ? "true" : "false") << " (Expected: true)" << endl << endl;
    
    // Test Case 2: Incorrect password
    cout << "Test 2 - Incorrect password:" << endl;
    bool result2 = verifyPassword("password.txt", "WrongPass123");
    cout << "Result: " << (result2 ? "true" : "false") << " (Expected: false)" << endl << endl;
    
    // Test Case 3: Password with different length
    cout << "Test 3 - Different length:" << endl;
    bool result3 = verifyPassword("password.txt", "Short");
    cout << "Result: " << (result3 ? "true" : "false") << " (Expected: false)" << endl << endl;
    
    // Test Case 4: Empty password
    cout << "Test 4 - Empty password:" << endl;
    bool result4 = verifyPassword("password.txt", "");
    cout << "Result: " << (result4 ? "true" : "false") << " (Expected: false)" << endl << endl;
    
    // Test Case 5: Case sensitive check
    cout << "Test 5 - Case sensitive:" << endl;
    bool result5 = verifyPassword("password.txt", "securepass123");
    cout << "Result: " << (result5 ? "true" : "false") << " (Expected: false)" << endl << endl;
    
    return 0;
}
