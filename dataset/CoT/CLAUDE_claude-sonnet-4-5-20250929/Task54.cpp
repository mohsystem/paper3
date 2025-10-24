
#include <iostream>
#include <string>
#include <map>
#include <random>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <openssl/sha.h>

class Task54 {
private:
    static const int OTP_LENGTH = 6;
    static const int OTP_VALIDITY_SECONDS = 300;
    
    struct OTPData {
        std::string hashedOTP;
        long long expiryTime;
        int attempts;
        
        OTPData() : attempts(0), expiryTime(0) {}
        OTPData(const std::string& hash, long long expiry) 
            : hashedOTP(hash), expiryTime(expiry), attempts(0) {}
    };
    
    std::map<std::string, OTPData> otpStorage;
    std::map<std::string, std::string> userCredentials;
    std::random_device rd;
    std::mt19937 gen;
    
public:
    Task54() : gen(rd()) {}
    
    std::string generateOTP() {
        std::uniform_int_distribution<> dis(0, 9);
        std::string otp;
        for (int i = 0; i < OTP_LENGTH; i++) {
            otp += std::to_string(dis(gen));
        }
        return otp;
    }
    
    std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
    void registerUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.length() < 8) {
            throw std::invalid_argument("Invalid username or password");
        }
        std::string hashedPassword = hashPassword(password);
        userCredentials[username] = hashedPassword;
    }
    
    bool authenticateUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return false;
        }
        
        auto it = userCredentials.find(username);
        if (it == userCredentials.end()) {
            return false;
        }
        
        std::string inputHash = hashPassword(password);
        return it->second == inputHash;
    }
    
    std::string sendOTP(const std::string& username) {
        if (userCredentials.find(username) == userCredentials.end()) {
            throw std::invalid_argument("User not found");
        }
        
        std::string otp = generateOTP();
        std::string hashedOTP = hashPassword(otp);
        
        auto now = std::chrono::system_clock::now();
        long long expiryTime = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count() + OTP_VALIDITY_SECONDS;
        
        otpStorage[username] = OTPData(hashedOTP, expiryTime);
        
        return otp;
    }
    
    bool verifyOTP(const std::string& username, const std::string& otp) {
        if (username.empty() || otp.empty()) {
            return false;
        }
        
        auto it = otpStorage.find(username);
        if (it == otpStorage.end()) {
            return false;
        }
        
        OTPData& otpData = it->second;
        
        if (otpData.attempts >= 3) {
            otpStorage.erase(username);
            return false;
        }
        
        otpData.attempts++;
        
        auto now = std::chrono::system_clock::now();
        long long currentTime = std::chrono::duration_cast<std::chrono::seconds>(
            now.time_since_epoch()).count();
        
        if (currentTime > otpData.expiryTime) {
            otpStorage.erase(username);
            return false;
        }
        
        std::string hashedInputOTP = hashPassword(otp);
        bool isValid = (otpData.hashedOTP == hashedInputOTP);
        
        if (isValid) {
            otpStorage.erase(username);
        }
        
        return isValid;
    }
    
    bool twoFactorLogin(const std::string& username, const std::string& password, 
                        const std::string& otp) {
        if (!authenticateUser(username, password)) {
            return false;
        }
        return verifyOTP(username, otp);
    }
};

int main() {
    std::cout << "Two-Factor Authentication System Test Cases\\n\\n";
    
    // Test Case 1: Successful 2FA login
    std::cout << "Test Case 1: Successful 2FA login\\n";
    Task54 auth1;
    auth1.registerUser("user1", "SecurePass123");
    std::string otp1 = auth1.sendOTP("user1");
    bool result1 = auth1.twoFactorLogin("user1", "SecurePass123", otp1);
    std::cout << "Result: " << (result1 ? "SUCCESS" : "FAILED") << "\\n\\n";
    
    // Test Case 2: Wrong password
    std::cout << "Test Case 2: Wrong password\\n";
    Task54 auth2;
    auth2.registerUser("user2", "SecurePass456");
    std::string otp2 = auth2.sendOTP("user2");
    bool result2 = auth2.twoFactorLogin("user2", "WrongPassword", otp2);
    std::cout << "Result: " << (result2 ? "SUCCESS" : "FAILED (Expected)") << "\\n\\n";
    
    // Test Case 3: Wrong OTP
    std::cout << "Test Case 3: Wrong OTP\\n";
    Task54 auth3;
    auth3.registerUser("user3", "SecurePass789");
    auth3.sendOTP("user3");
    bool result3 = auth3.twoFactorLogin("user3", "SecurePass789", "000000");
    std::cout << "Result: " << (result3 ? "SUCCESS" : "FAILED (Expected)") << "\\n\\n";
    
    // Test Case 4: Valid OTP verification
    std::cout << "Test Case 4: Valid OTP verification\\n";
    Task54 auth4;
    auth4.registerUser("user4", "SecurePass101");
    std::string otp4 = auth4.sendOTP("user4");
    bool result4 = auth4.verifyOTP("user4", otp4);
    std::cout << "Result: " << (result4 ? "SUCCESS" : "FAILED") << "\\n\\n";
    
    // Test Case 5: Multiple users
    std::cout << "Test Case 5: Multiple users handling\\n";
    Task54 auth5;
    auth5.registerUser("user5", "SecurePass202");
    auth5.registerUser("user6", "SecurePass303");
    std::string otp5 = auth5.sendOTP("user5");
    std::string otp6 = auth5.sendOTP("user6");
    bool result5a = auth5.twoFactorLogin("user5", "SecurePass202", otp5);
    bool result5b = auth5.twoFactorLogin("user6", "SecurePass303", otp6);
    std::cout << "User5 Result: " << (result5a ? "SUCCESS" : "FAILED") << "\\n";
    std::cout << "User6 Result: " << (result5b ? "SUCCESS" : "FAILED") << "\\n";
    
    return 0;
}
