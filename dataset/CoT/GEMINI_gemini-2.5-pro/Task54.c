#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// For boolean type
#include <stdbool.h>

#define MAX_USERS 2
#define USERNAME_LEN 32
#define PASSWORD_LEN 32
#define OTP_LEN 7 // 6 digits + null terminator

typedef struct {
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
} User;

// IMPORTANT: In a real-world application, never store passwords in plaintext.
// They should be securely hashed and salted. This is for demonstration purposes only.
User userDatabase[MAX_USERS] = {
    {"alice", "password123"},
    {"bob", "bob@secret"}
};

/**
 * Verifies user credentials against the stored database.
 * @param username The username to check.
 * @param password The password to check.
 * @return true if credentials are valid, false otherwise.
 */
bool verifyCredentials(const char* username, const char* password) {
    for (int i = 0; i < MAX_USERS; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            if (strcmp(userDatabase[i].password, password) == 0) {
                return true;
            }
        }
    }
    return false;
}

/**
 * Generates a 6-digit One-Time Password (OTP).
 * NOTE: rand() is not cryptographically secure. For a real application,
 * use a platform-specific secure random number generator like /dev/urandom on Linux
 * or CryptGenRandom on Windows.
 * @param otp_buffer A character buffer to store the generated OTP.
 * @param buffer_size The size of the buffer (must be at least OTP_LEN).
 */
void generateOTP(char* otp_buffer, size_t buffer_size) {
    if (buffer_size < OTP_LEN) {
        return; // Buffer too small
    }
    // Generate a number between 100000 and 999999
    int otp_num = 100000 + (rand() % 900000);
    snprintf(otp_buffer, buffer_size, "%06d", otp_num);
}

/**
 * Verifies if the user-provided OTP matches the generated OTP.
 * @param generatedOTP The system-generated OTP.
 * @param userInputOTP The OTP entered by the user.
 * @return true if the OTPs match, false otherwise.
 */
bool verifyOTP(const char* generatedOTP, const char* userInputOTP) {
    if (generatedOTP == NULL || userInputOTP == NULL) {
        return false;
    }
    return strcmp(generatedOTP, userInputOTP) == 0;
}

/**
 * Simulates the full login process for a given user.
 * @param username The user's username.
 * @param password The user's password.
 * @param otpInput The OTP provided by the user for the simulation.
 */
void loginProcess(const char* username, const char* password, const char* otpInput) {
    printf("--- Attempting login for user: %s ---\n", username);
    if (verifyCredentials(username, password)) {
        printf("Credentials verified. Generating OTP...\n");
        char otp[OTP_LEN];
        generateOTP(otp, sizeof(otp));
        
        // In a real application, this OTP would be sent to the user via SMS, email, etc.
        // For this simulation, we will print it to the console.
        printf("Generated OTP (for simulation): %s\n", otp);
        printf("User provided OTP: %s\n", otpInput);

        if (verifyOTP(otp, otpInput)) {
            printf("OTP verification successful. Login successful!\n");
        } else {
            printf("OTP verification failed. Access denied.\n");
        }
    } else {
        printf("Invalid username or password. Access denied.\n");
    }
    printf("-----------------------------------------\n\n");
}

int main() {
    // Seed the random number generator. Should only be done once.
    srand((unsigned int)time(NULL));

    // Test Case 1: Successful login
    // Simulate correct password and then providing the correct OTP.
    // For the test, we'll "peek" at the OTP to simulate a user receiving and entering it.
    const char* correctUsername1 = "alice";
    const char* correctPassword1 = "password123";
    if (verifyCredentials(correctUsername1, correctPassword1)) {
        char generatedOtp1[OTP_LEN];
        generateOTP(generatedOtp1, sizeof(generatedOtp1)); // We generate it to pass it to the simulation
        loginProcess(correctUsername1, correctPassword1, generatedOtp1);
    }

    // Test Case 2: Incorrect password
    loginProcess("alice", "wrongpassword", "123456");

    // Test Case 3: Correct password, incorrect OTP
    loginProcess("bob", "bob@secret", "000000");

    // Test Case 4: Non-existent user
    loginProcess("charlie", "somepassword", "654321");
    
    // Test Case 5: Another successful login
    const char* correctUsername2 = "bob";
    const char* correctPassword2 = "bob@secret";
    if (verifyCredentials(correctUsername2, correctPassword2)) {
        char generatedOtp2[OTP_LEN];
        generateOTP(generatedOtp2, sizeof(generatedOtp2));
        loginProcess(correctUsername2, correctPassword2, generatedOtp2);
    }

    return 0;
}