#include <iostream>
#include <string>
#include <map>
#include <random>
#include <iomanip>
#include <sstream>

// In-memory storage for user credentials.
// WARNING: Storing plain-text passwords is insecure. Use hashed passwords in production.
std::map<std::string, std::string> userDatabase = {
    {"alice", "password123"},
    {"bob", "bob@1234"}
};

// In-memory storage for one-time passwords.
std::map<std::string, std::string> otpStorage;

/**
 * @brief Factor 1: Validates username and password.
 * 
 * @param username The user's username.
 * @param password The user's password.
 * @return true if credentials are valid, false otherwise.
 */
bool login(const std::string& username, const std::string& password) {
    auto it = userDatabase.find(username);
    if (it != userDatabase.end()) {
        return it->second == password;
    }
    return false;
}

/**
 * @brief Generates and "sends" a 6-digit OTP for the user.
 * In a real application, this would be sent via SMS or email.
 * 
 * @param username The user to generate an OTP for.
 * @return The generated OTP string.
 */
std::string generateAndSendOTP(const std::string& username) {
    if (userDatabase.find(username) == userDatabase.end()) {
        return "";
    }

    // Use a random_device for a non-deterministic seed, which is more secure.
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(100000, 999999);
    
    int otpValue = distrib(gen);
    
    std::stringstream ss;
    ss << std::setw(6) << std::setfill('0') << otpValue;
    std::string otpString = ss.str();

    otpStorage[username] = otpString;

    std::cout << "OTP sent to " << username << ". Your OTP is: " << otpString << std::endl;
    return otpString;
}

/**
 * @brief Factor 2: Validates the provided OTP for the user.
 * 
 * @param username The user's username.
 * @param otp The one-time password provided by the user.
 * @return true if the OTP is valid, false otherwise.
 */
bool validateOTP(const std::string& username, const std::string& otp) {
    auto it = otpStorage.find(username);
    if (it != otpStorage.end() && it->second == otp) {
        // Invalidate the OTP after use
        otpStorage.erase(it);
        return true;
    }
    return false;
}

int main() {
    std::cout << "--- Running 2FA Login Test Cases ---" << std::endl;

    // Test Case 1: Successful Login
    std::cout << "\n--- Test Case 1: Successful Login ---" << std::endl;
    std::string user1 = "alice";
    std::string pass1 = "password123";
    std::cout << "Attempting to log in user: " << user1 << std::endl;
    if (login(user1, pass1)) {
        std::cout << "Step 1 (Password) successful." << std::endl;
        std::string generatedOtp1 = generateAndSendOTP(user1);
        if (validateOTP(user1, generatedOtp1)) {
            std::cout << "Step 2 (OTP) successful. Login successful for " << user1 << "!" << std::endl;
        } else {
            std::cout << "Step 2 (OTP) failed. Login failed." << std::endl;
        }
    } else {
        std::cout << "Step 1 (Password) failed. Invalid username or password." << std::endl;
    }

    // Test Case 2: Invalid Password
    std::cout << "\n--- Test Case 2: Invalid Password ---" << std::endl;
    std::string user2 = "bob";
    std::string pass2 = "wrongpassword";
    std::cout << "Attempting to log in user: " << user2 << std::endl;
    if (login(user2, pass2)) {
        std::cout << "Step 1 (Password) successful." << std::endl;
        generateAndSendOTP(user2);
    } else {
        std::cout << "Step 1 (Password) failed. Invalid username or password." << std::endl;
    }

    // Test Case 3: Invalid Username
    std::cout << "\n--- Test Case 3: Invalid Username ---" << std::endl;
    std::string user3 = "charlie";
    std::string pass3 = "password123";
    std::cout << "Attempting to log in user: " << user3 << std::endl;
    if (login(user3, pass3)) {
        std::cout << "Step 1 (Password) successful." << std::endl;
    } else {
        std::cout << "Step 1 (Password) failed. Invalid username or password." << std::endl;
    }

    // Test Case 4: Incorrect OTP
    std::cout << "\n--- Test Case 4: Incorrect OTP ---" << std::endl;
    std::string user4 = "bob";
    std::string pass4 = "bob@1234";
    std::cout << "Attempting to log in user: " << user4 << std::endl;
    if (login(user4, pass4)) {
        std::cout << "Step 1 (Password) successful." << std::endl;
        generateAndSendOTP(user4);
        std::string incorrectOtp = "111111";
        std::cout << "User provides OTP: " << incorrectOtp << std::endl;
        if (validateOTP(user4, incorrectOtp)) {
            std::cout << "Step 2 (OTP) successful. Login successful for " << user4 << "!" << std::endl;
        } else {
            std::cout << "Step 2 (OTP) failed. Login failed." << std::endl;
        }
    } else {
        std::cout << "Step 1 (Password) failed. Invalid username or password." << std::endl;
    }
    
    // Test Case 5: Another Successful Login
    std::cout << "\n--- Test Case 5: Another Successful Login ---" << std::endl;
    std::string user5 = "bob";
    std::string pass5 = "bob@1234";
    std::cout << "Attempting to log in user: " << user5 << std::endl;
    if (login(user5, pass5)) {
        std::cout << "Step 1 (Password) successful." << std::endl;
        std::string generatedOtp5 = generateAndSendOTP(user5);
        if (validateOTP(user5, generatedOtp5)) {
            std::cout << "Step 2 (OTP) successful. Login successful for " << user5 << "!" << std::endl;
        } else {
            std::cout << "Step 2 (OTP) failed. Login failed." << std::endl;
        }
    } else {
        std::cout << "Step 1 (Password) failed. Invalid username or password." << std::endl;
    }

    return 0;
}