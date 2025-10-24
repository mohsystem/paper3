
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <ctime>
#include <chrono>

class Task54 {
private:
    std::map<std::string, std::string> userDatabase;
    std::map<std::string, std::string> otpStorage;
    std::map<std::string, long long> otpExpiry;
    static const int OTP_LENGTH = 6;
    static const long long OTP_VALIDITY = 300000; // 5 minutes in milliseconds
    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dis;

public:
    Task54() : gen(rd()), dis(0, 9) {}

    std::string generateOTP() {
        std::string otp = "";
        for (int i = 0; i < OTP_LENGTH; i++) {
            otp += std::to_string(dis(gen));
        }
        return otp;
    }

    bool registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        if (userDatabase.find(username) != userDatabase.end()) {
            return false;
        }
        userDatabase[username] = password;
        return true;
    }

    std::string authenticateAndGenerateOTP(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "";
        }
        if (userDatabase.find(username) == userDatabase.end()) {
            return "";
        }
        if (userDatabase[username] != password) {
            return "";
        }
        
        std::string otp = generateOTP();
        long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        
        otpStorage[username] = otp;
        otpExpiry[username] = currentTime + OTP_VALIDITY;
        return otp;
    }

    bool verifyOTP(const std::string& username, const std::string& otp) {
        if (username.empty() || otp.empty()) {
            return false;
        }
        if (otpStorage.find(username) == otpStorage.end()) {
            return false;
        }
        
        long long currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        
        if (currentTime > otpExpiry[username]) {
            otpStorage.erase(username);
            otpExpiry.erase(username);
            return false;
        }
        
        bool isValid = (otpStorage[username] == otp);
        if (isValid) {
            otpStorage.erase(username);
            otpExpiry.erase(username);
        }
        return isValid;
    }
};

int main() {
    std::cout << "=== Two-Factor Authentication Test Cases ===\\n\\n";

    // Test Case 1: Register and successful 2FA
    std::cout << "Test Case 1: Successful Registration and 2FA\\n";
    Task54 auth1;
    auth1.registerUser("user1", "password123");
    std::string otp1 = auth1.authenticateAndGenerateOTP("user1", "password123");
    std::cout << "Generated OTP: " << otp1 << "\\n";
    bool result1 = auth1.verifyOTP("user1", otp1);
    std::cout << "2FA Result: " << (result1 ? "SUCCESS" : "FAILED") << "\\n\\n";

    // Test Case 2: Invalid password
    std::cout << "Test Case 2: Invalid Password\\n";
    Task54 auth2;
    auth2.registerUser("user2", "securepass");
    std::string otp2 = auth2.authenticateAndGenerateOTP("user2", "wrongpass");
    std::cout << "OTP Generation: " << (otp2.empty() ? "FAILED (Invalid credentials)" : "SUCCESS") << "\\n\\n";

    // Test Case 3: Wrong OTP
    std::cout << "Test Case 3: Wrong OTP\\n";
    Task54 auth3;
    auth3.registerUser("user3", "mypassword");
    std::string otp3 = auth3.authenticateAndGenerateOTP("user3", "mypassword");
    std::cout << "Generated OTP: " << otp3 << "\\n";
    bool result3 = auth3.verifyOTP("user3", "000000");
    std::cout << "2FA with wrong OTP: " << (result3 ? "SUCCESS" : "FAILED") << "\\n\\n";

    // Test Case 4: Reusing OTP
    std::cout << "Test Case 4: Reusing OTP\\n";
    Task54 auth4;
    auth4.registerUser("user4", "pass456");
    std::string otp4 = auth4.authenticateAndGenerateOTP("user4", "pass456");
    std::cout << "Generated OTP: " << otp4 << "\\n";
    bool result4a = auth4.verifyOTP("user4", otp4);
    std::cout << "First verification: " << (result4a ? "SUCCESS" : "FAILED") << "\\n";
    bool result4b = auth4.verifyOTP("user4", otp4);
    std::cout << "Second verification (reuse): " << (result4b ? "SUCCESS" : "FAILED") << "\\n\\n";

    // Test Case 5: Non-existent user
    std::cout << "Test Case 5: Non-existent User\\n";
    Task54 auth5;
    std::string otp5 = auth5.authenticateAndGenerateOTP("nonexistent", "password");
    std::cout << "OTP Generation for non-existent user: " << (otp5.empty() ? "FAILED (User not found)" : "SUCCESS") << "\\n\\n";

    return 0;
}
