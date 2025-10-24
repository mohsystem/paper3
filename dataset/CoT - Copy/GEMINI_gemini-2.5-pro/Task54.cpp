#include <iostream>
#include <string>
#include <map>
#include <random>
#include <iomanip>
#include <sstream>

class Task54 {
public:
    // IMPORTANT: In a real-world application, never store passwords in plaintext.
    // They should be securely hashed and salted. This is for demonstration purposes only.
    static std::map<std::string, std::string> userDatabase;

    /**
     * Verifies user credentials against the stored database.
     * @param username The username to check.
     * @param password The password to check.
     * @return true if credentials are valid, false otherwise.
     */
    static bool verifyCredentials(const std::string& username, const std::string& password) {
        auto it = userDatabase.find(username);
        if (it != userDatabase.end()) {
            return it->second == password;
        }
        return false;
    }

    /**
     * Generates a secure 6-digit One-Time Password (OTP).
     * @return A 6-digit OTP as a std::string.
     */
    static std::string generateOTP() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(100000, 999999);
        
        std::stringstream ss;
        ss << std::setw(6) << std::setfill('0') << distrib(gen);
        return ss.str();
    }

    /**
     * Verifies if the user-provided OTP matches the generated OTP.
     * @param generatedOTP The system-generated OTP.
     * @param userInputOTP The OTP entered by the user.
     * @return true if the OTPs match, false otherwise.
     */
    static bool verifyOTP(const std::string& generatedOTP, const std::string& userInputOTP) {
        return generatedOTP == userInputOTP;
    }

    /**
     * Simulates the full login process for a given user.
     * @param username The user's username.
     * @param password The user's password.
     * @param otpInput The OTP provided by the user for the simulation.
     */
    static void loginProcess(const std::string& username, const std::string& password, const std::string& otpInput) {
        std::cout << "--- Attempting login for user: " << username << " ---" << std::endl;
        if (verifyCredentials(username, password)) {
            std::cout << "Credentials verified. Generating OTP..." << std::endl;
            std::string otp = generateOTP();
            
            // In a real application, this OTP would be sent to the user via SMS, email, etc.
            // For this simulation, we will print it to the console.
            std::cout << "Generated OTP (for simulation): " << otp << std::endl;
            std::cout << "User provided OTP: " << otpInput << std::endl;

            if (verifyOTP(otp, otpInput)) {
                std::cout << "OTP verification successful. Login successful!" << std::endl;
            } else {
                std::cout << "OTP verification failed. Access denied." << std::endl;
            }
        } else {
            std::cout << "Invalid username or password. Access denied." << std::endl;
        }
        std::cout << "-----------------------------------------\n" << std::endl;
    }
};

// Initialize the static user database
std::map<std::string, std::string> Task54::userDatabase = {
    {"alice", "password123"},
    {"bob", "bob@secret"}
};

int main() {
    // Test Case 1: Successful login
    // Simulate correct password and then providing the correct OTP.
    // For the test, we'll "peek" at the OTP to simulate a user receiving and entering it.
    std::string correctUsername1 = "alice";
    std::string correctPassword1 = "password123";
    if (Task54::verifyCredentials(correctUsername1, correctPassword1)) {
        std::string generatedOtp1 = Task54::generateOTP(); // We generate it to pass it to the simulation
        Task54::loginProcess(correctUsername1, correctPassword1, generatedOtp1);
    }

    // Test Case 2: Incorrect password
    Task54::loginProcess("alice", "wrongpassword", "123456");

    // Test Case 3: Correct password, incorrect OTP
    Task54::loginProcess("bob", "bob@secret", "000000");

    // Test Case 4: Non-existent user
    Task54::loginProcess("charlie", "somepassword", "654321");

    // Test Case 5: Another successful login
    std::string correctUsername2 = "bob";
    std::string correctPassword2 = "bob@secret";
    if (Task54::verifyCredentials(correctUsername2, correctPassword2)) {
        std::string generatedOtp2 = Task54::generateOTP();
        Task54::loginProcess(correctUsername2, correctPassword2, generatedOtp2);
    }

    return 0;
}