#include <iostream>
#include <string>
#include <map>
#include <random>
#include <vector>
#include <algorithm>
#include <limits>

// In a real application, passwords should be hashed and salted using a strong
// algorithm like Argon2. This is a simplified example focusing on OTP generation.
// Also, users would be stored in a secure database, not hardcoded.
std::map<std::string, std::string> user_database = {
    {"alice", "password123"},
    {"bob", "securepass"}
};

// Generates a cryptographically secure random OTP of a given length.
std::string generate_otp(size_t length) {
    if (length == 0) {
        return "";
    }
    std::string otp;
    otp.reserve(length);
    // std::random_device is a source of non-deterministic random numbers, suitable for seeding.
    std::random_device rd; 
    // Seed the Mersenne Twister engine.
    std::mt19937 gen(rd()); 
    std::uniform_int_distribution<> distrib(0, 9);

    for (size_t i = 0; i < length; ++i) {
        otp += std::to_string(distrib(gen));
    }
    return otp;
}

bool verify_login(const std::string& username, const std::string& password) {
    auto it = user_database.find(username);
    if (it != user_database.end() && it->second == password) {
        return true;
    }
    return false;
}

bool perform_2fa() {
    std::string username, password, entered_otp;

    std::cout << "Enter username: ";
    if (!std::getline(std::cin, username)) {
        return false; // Handle EOF or input error
    }

    std::cout << "Enter password: ";
    if (!std::getline(std::cin, password)) {
        return false; // Handle EOF or input error
    }

    if (verify_login(username, password)) {
        std::cout << "Password correct. 2FA required." << std::endl;
        std::string otp = generate_otp(6);
        
        // In a real system, this OTP would be sent via SMS, email, or an authenticator app.
        std::cout << "Generated OTP (for simulation): " << otp << std::endl;
        
        std::cout << "Enter the 6-digit OTP: ";
        if (!std::getline(std::cin, entered_otp)) {
             return false; // Handle EOF or input error
        }

        if (entered_otp == otp) {
            std::cout << "Login successful!" << std::endl;
            return true;
        } else {
            std::cout << "Invalid OTP. Login failed." << std::endl;
            return false;
        }
    } else {
        std::cout << "Invalid username or password." << std::endl;
        return false;
    }
}


void run_test_case(int case_num) {
    std::cout << "\n--- Test Case " << case_num << " ---" << std::endl;
    switch(case_num) {
        case 1:
            std::cout << "Goal: Successful login." << std::endl;
            std::cout << "To pass: Enter username 'alice' and password 'password123'." << std::endl;
            std::cout << "Then enter the generated OTP correctly." << std::endl;
            break;
        case 2:
            std::cout << "Goal: Fail login with incorrect password." << std::endl;
            std::cout << "To pass: Enter username 'alice' and password 'wrongpassword'." << std::endl;
            break;
        case 3:
            std::cout << "Goal: Fail login with non-existent user." << std::endl;
            std::cout << "To pass: Enter username 'charlie' and any password." << std::endl;
            break;
        case 4:
            std::cout << "Goal: Fail login with incorrect OTP." << std::endl;
            std::cout << "To pass: Enter username 'bob' and password 'securepass'." << std::endl;
            std::cout << "Then enter an incorrect OTP (e.g., '000000')." << std::endl;
            break;
        case 5:
            std::cout << "Goal: Successful login again to show OTP is random." << std::endl;
            std::cout << "To pass: Enter username 'alice' and password 'password123' again." << std::endl;
            std::cout << "Observe that the OTP is different from Test Case 1." << std::endl;
            break;
    }
    perform_2fa();
}

int main() {
    std::cout << "2FA Login Simulation\n";
    std::cout << "====================\n";

    run_test_case(1);
    run_test_case(2);
    run_test_case(3);
    run_test_case(4);
    run_test_case(5);

    std::cout << "\n--- All test cases completed. ---" << std::endl;
    
    return 0;
}