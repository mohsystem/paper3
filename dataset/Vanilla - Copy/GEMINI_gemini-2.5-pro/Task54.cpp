#include <iostream>
#include <string>
#include <map>
#include <random>
#include <ctime>

// In-memory storage for user credentials (username -> password)
std::map<std::string, std::string> users;
// In-memory storage for generated OTPs (username -> otp)
std::map<std::string, std::string> userOtps;

/**
 * Verifies the user's primary credentials (username and password).
 * @param username The username.
 * @param password The password.
 * @return true if credentials are valid, false otherwise.
 */
bool verifyUser(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it != users.end()) {
        return it->second == password;
    }
    return false;
}

/**
 * Generates a random 6-digit OTP, stores it, and returns it.
 * @param username The username for whom to generate the OTP.
 * @return The generated 6-digit OTP as a string.
 */
std::string generateAndSendOtp(const std::string& username) {
    if (users.count(username)) {
        static std::mt19937 rng(time(0)); // Seeded once
        std::uniform_int_distribution<int> dist(100000, 999999);
        std::string otp = std::to_string(dist(rng));
        userOtps[username] = otp;
        // Simulate sending OTP to the user
        std::cout << "OTP sent for user '" << username << "'. OTP is: " << otp << " (for simulation purposes)" << std::endl;
        return otp;
    }
    return "";
}

/**
 * Verifies the entered OTP against the stored OTP for a user.
 * @param username The username.
 * @param enteredOtp The OTP entered by the user.
 * @return true if the OTP is correct, false otherwise.
 */
bool verifyOtp(const std::string& username, const std::string& enteredOtp) {
    auto it = userOtps.find(username);
    if (it != userOtps.end() && it->second == enteredOtp) {
        // OTP is single-use, remove it after verification
        userOtps.erase(it);
        return true;
    }
    return false;
}

int main() {
    // Populate user data
    users["user1"] = "pass123";
    users["user2"] = "secret456";
    users["user3"] = "qwerty";

    // --- Test Case 1: Successful Login ---
    std::cout << "--- Test Case 1: Successful Login ---" << std::endl;
    std::string user1 = "user1";
    std::string pass1 = "pass123";
    if (verifyUser(user1, pass1)) {
        std::cout << "Primary authentication successful for " << user1 << std::endl;
        std::string otp1 = generateAndSendOtp(user1);
        // Simulate user entering the correct OTP
        if (verifyOtp(user1, otp1)) {
            std::cout << "2FA successful. Login complete for " << user1 << std::endl;
        } else {
            std::cout << "2FA failed. Invalid OTP." << std::endl;
        }
    } else {
        std::cout << "Primary authentication failed for " << user1 << std::endl;
    }
    std::cout << "\n----------------------------------------\n" << std::endl;

    // --- Test Case 2: Incorrect Password ---
    std::cout << "--- Test Case 2: Incorrect Password ---" << std::endl;
    std::string user2 = "user2";
    std::string pass2_wrong = "wrongpassword";
    if (verifyUser(user2, pass2_wrong)) {
        std::cout << "Primary authentication successful for " << user2 << std::endl;
        generateAndSendOtp(user2);
    } else {
        std::cout << "Primary authentication failed for " << user2 << ". Incorrect username or password." << std::endl;
    }
    std::cout << "\n----------------------------------------\n" << std::endl;

    // --- Test Case 3: Incorrect Username ---
    std::cout << "--- Test Case 3: Incorrect Username ---" << std::endl;
    std::string user3_wrong = "nonexistentuser";
    std::string pass3 = "qwerty";
    if (verifyUser(user3_wrong, pass3)) {
        std::cout << "Primary authentication successful for " << user3_wrong << std::endl;
        generateAndSendOtp(user3_wrong);
    } else {
        std::cout << "Primary authentication failed for " << user3_wrong << ". Incorrect username or password." << std::endl;
    }
    std::cout << "\n----------------------------------------\n" << std::endl;

    // --- Test Case 4: Correct Password, Incorrect OTP ---
    std::cout << "--- Test Case 4: Correct Password, Incorrect OTP ---" << std::endl;
    std::string user4 = "user3";
    std::string pass4 = "qwerty";
    if (verifyUser(user4, pass4)) {
        std::cout << "Primary authentication successful for " << user4 << std::endl;
        generateAndSendOtp(user4);
        // Simulate user entering a wrong OTP
        std::string wrongOtp = "000000";
        std::cout << "User '" << user4 << "' enters OTP: " << wrongOtp << std::endl;
        if (verifyOtp(user4, wrongOtp)) {
            std::cout << "2FA successful. Login complete for " << user4 << std::endl;
        } else {
            std::cout << "2FA failed. Invalid OTP for " << user4 << std::endl;
        }
    } else {
        std::cout << "Primary authentication failed for " << user4 << std::endl;
    }
    std::cout << "\n----------------------------------------\n" << std::endl;

    // --- Test Case 5: Another Successful Login ---
    std::cout << "--- Test Case 5: Another Successful Login ---" << std::endl;
    std::string user5 = "user2";
    std::string pass5 = "secret456";
    if (verifyUser(user5, pass5)) {
        std::cout << "Primary authentication successful for " << user5 << std::endl;
        std::string otp5 = generateAndSendOtp(user5);
        // Simulate user entering the correct OTP
        if (verifyOtp(user5, otp5)) {
            std::cout << "2FA successful. Login complete for " << user5 << std::endl;
        } else {
            std::cout << "2FA failed. Invalid OTP." << std::endl;
        }
    } else {
        std::cout << "Primary authentication failed for " << user5 << std::endl;
    }
    std::cout << "\n----------------------------------------\n" << std::endl;

    return 0;
}