
#include <iostream>
#include <map>
#include <string>
#include <ctime>
#include <cstdlib>
#include <chrono>

using namespace std;

class Task54 {
private:
    struct User {
        string username;
        string password;
        string email;
        
        User() {}
        User(string u, string p, string e) : username(u), password(p), email(e) {}
    };
    
    struct OTP {
        string code;
        chrono::system_clock::time_point generatedAt;
        
        OTP() {}
        OTP(string c, chrono::system_clock::time_point t) : code(c), generatedAt(t) {}
        
        bool isExpired(int expiryMinutes) {
            auto now = chrono::system_clock::now();
            auto duration = chrono::duration_cast<chrono::minutes>(now - generatedAt);
            return duration.count() >= expiryMinutes;
        }
    };
    
    map<string, User> users;
    map<string, OTP> otpStore;
    static const int OTP_EXPIRY_MINUTES = 5;
    static const int OTP_LENGTH = 6;
    
public:
    string generateOTP() {
        string otp = "";
        for (int i = 0; i < OTP_LENGTH; i++) {
            otp += to_string(rand() % 10);
        }
        return otp;
    }
    
    bool registerUser(string username, string password, string email) {
        if (users.find(username) != users.end()) {
            return false;
        }
        users[username] = User(username, password, email);
        return true;
    }
    
    string authenticateCredentials(string username, string password) {
        if (users.find(username) == users.end() || users[username].password != password) {
            return "";
        }
        
        string otp = generateOTP();
        otpStore[username] = OTP(otp, chrono::system_clock::now());
        return otp;
    }
    
    bool verifyOTP(string username, string enteredOTP) {
        if (otpStore.find(username) == otpStore.end()) {
            return false;
        }
        
        OTP& otp = otpStore[username];
        
        if (otp.isExpired(OTP_EXPIRY_MINUTES)) {
            otpStore.erase(username);
            return false;
        }
        
        if (otp.code == enteredOTP) {
            otpStore.erase(username);
            return true;
        }
        
        return false;
    }
    
    string performLogin(string username, string password, string enteredOTP) {
        string generatedOTP = authenticateCredentials(username, password);
        if (generatedOTP.empty()) {
            return "Login Failed: Invalid credentials";
        }
        
        if (verifyOTP(username, enteredOTP)) {
            return "Login Successful";
        } else {
            return "Login Failed: Invalid or expired OTP";
        }
    }
};

int main() {
    srand(time(0));
    Task54 authSystem;
    
    cout << "=== Two-Factor Authentication System ===" << endl << endl;
    
    // Test Case 1: Successful registration and login
    cout << "Test Case 1: Successful registration and login" << endl;
    authSystem.registerUser("user1", "pass123", "user1@example.com");
    string otp1 = authSystem.authenticateCredentials("user1", "pass123");
    cout << "Generated OTP: " << otp1 << endl;
    string result1 = authSystem.performLogin("user1", "pass123", otp1);
    cout << "Result: " << result1 << endl << endl;
    
    // Test Case 2: Wrong password
    cout << "Test Case 2: Wrong password" << endl;
    authSystem.registerUser("user2", "pass456", "user2@example.com");
    string result2 = authSystem.performLogin("user2", "wrongpass", "123456");
    cout << "Result: " << result2 << endl << endl;
    
    // Test Case 3: Wrong OTP
    cout << "Test Case 3: Wrong OTP" << endl;
    authSystem.registerUser("user3", "pass789", "user3@example.com");
    string otp3 = authSystem.authenticateCredentials("user3", "pass789");
    cout << "Generated OTP: " << otp3 << endl;
    string result3 = authSystem.performLogin("user3", "pass789", "000000");
    cout << "Result: " << result3 << endl << endl;
    
    // Test Case 4: User doesn't exist\n    cout << "Test Case 4: User doesn't exist" << endl;\n    string result4 = authSystem.performLogin("nonexistent", "pass000", "123456");\n    cout << "Result: " << result4 << endl << endl;\n    \n    // Test Case 5: Multiple users with successful login\n    cout << "Test Case 5: Multiple users with successful login" << endl;\n    authSystem.registerUser("user4", "secure123", "user4@example.com");\n    authSystem.registerUser("user5", "secure456", "user5@example.com");\n    string otp4 = authSystem.authenticateCredentials("user4", "secure123");\n    string otp5 = authSystem.authenticateCredentials("user5", "secure456");\n    cout << "User4 OTP: " << otp4 << endl;\n    cout << "User5 OTP: " << otp5 << endl;\n    string result5a = authSystem.performLogin("user4", "secure123", otp4);\n    string result5b = authSystem.performLogin("user5", "secure456", otp5);\n    cout << "User4 Result: " << result5a << endl;\n    cout << "User5 Result: " << result5b << endl;
    
    return 0;
}
