
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_USERS 100
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define OTP_LENGTH 6
#define OTP_VALIDITY 300 // 5 minutes in seconds

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} User;

typedef struct {
    char username[MAX_USERNAME_LEN];
    char otp[OTP_LENGTH + 1];
    time_t expiry;
} OTPEntry;

User userDatabase[MAX_USERS];
OTPEntry otpStorage[MAX_USERS];
int userCount = 0;
int otpCount = 0;

void generateOTP(char* otp) {
    for (int i = 0; i < OTP_LENGTH; i++) {
        otp[i] = '0' + (rand() % 10);
    }
    otp[OTP_LENGTH] = '\\0';
}

bool registerUser(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return false;
    }
    
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            return false;
        }
    }
    
    if (userCount >= MAX_USERS) {
        return false;
    }
    
    strncpy(userDatabase[userCount].username, username, MAX_USERNAME_LEN - 1);
    strncpy(userDatabase[userCount].password, password, MAX_PASSWORD_LEN - 1);
    userCount++;
    return true;
}

char* authenticateAndGenerateOTP(const char* username, const char* password, char* otpBuffer) {
    if (username == NULL || password == NULL) {
        return NULL;
    }
    
    int userIndex = -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            userIndex = i;
            break;
        }
    }
    
    if (userIndex == -1) {
        return NULL;
    }
    
    if (strcmp(userDatabase[userIndex].password, password) != 0) {
        return NULL;
    }
    
    generateOTP(otpBuffer);
    
    int otpIndex = -1;
    for (int i = 0; i < otpCount; i++) {
        if (strcmp(otpStorage[i].username, username) == 0) {
            otpIndex = i;
            break;
        }
    }
    
    if (otpIndex == -1) {
        if (otpCount < MAX_USERS) {
            otpIndex = otpCount++;
        } else {
            return NULL;
        }
    }
    
    strncpy(otpStorage[otpIndex].username, username, MAX_USERNAME_LEN - 1);
    strncpy(otpStorage[otpIndex].otp, otpBuffer, OTP_LENGTH);
    otpStorage[otpIndex].expiry = time(NULL) + OTP_VALIDITY;
    
    return otpBuffer;
}

bool verifyOTP(const char* username, const char* otp) {
    if (username == NULL || otp == NULL) {
        return false;
    }
    
    int otpIndex = -1;
    for (int i = 0; i < otpCount; i++) {
        if (strcmp(otpStorage[i].username, username) == 0) {
            otpIndex = i;
            break;
        }
    }
    
    if (otpIndex == -1) {
        return false;
    }
    
    if (time(NULL) > otpStorage[otpIndex].expiry) {
        for (int i = otpIndex; i < otpCount - 1; i++) {
            otpStorage[i] = otpStorage[i + 1];
        }
        otpCount--;
        return false;
    }
    
    bool isValid = (strcmp(otpStorage[otpIndex].otp, otp) == 0);
    if (isValid) {
        for (int i = otpIndex; i < otpCount - 1; i++) {
            otpStorage[i] = otpStorage[i + 1];
        }
        otpCount--;
    }
    return isValid;
}

int main() {
    srand(time(NULL));
    char otpBuffer[OTP_LENGTH + 1];
    
    printf("=== Two-Factor Authentication Test Cases ===\\n\\n");

    // Test Case 1: Register and successful 2FA
    printf("Test Case 1: Successful Registration and 2FA\\n");
    registerUser("user1", "password123");
    char* otp1 = authenticateAndGenerateOTP("user1", "password123", otpBuffer);
    printf("Generated OTP: %s\\n", otp1);
    bool result1 = verifyOTP("user1", otp1);
    printf("2FA Result: %s\\n\\n", result1 ? "SUCCESS" : "FAILED");

    // Test Case 2: Invalid password
    printf("Test Case 2: Invalid Password\\n");
    registerUser("user2", "securepass");
    char otpBuffer2[OTP_LENGTH + 1];
    char* otp2 = authenticateAndGenerateOTP("user2", "wrongpass", otpBuffer2);
    printf("OTP Generation: %s\\n\\n", otp2 == NULL ? "FAILED (Invalid credentials)" : "SUCCESS");

    // Test Case 3: Wrong OTP
    printf("Test Case 3: Wrong OTP\\n");
    registerUser("user3", "mypassword");
    char otpBuffer3[OTP_LENGTH + 1];
    char* otp3 = authenticateAndGenerateOTP("user3", "mypassword", otpBuffer3);
    printf("Generated OTP: %s\\n", otp3);
    bool result3 = verifyOTP("user3", "000000");
    printf("2FA with wrong OTP: %s\\n\\n", result3 ? "SUCCESS" : "FAILED");

    // Test Case 4: Reusing OTP
    printf("Test Case 4: Reusing OTP\\n");
    registerUser("user4", "pass456");
    char otpBuffer4[OTP_LENGTH + 1];
    char* otp4 = authenticateAndGenerateOTP("user4", "pass456", otpBuffer4);
    printf("Generated OTP: %s\\n", otp4);
    bool result4a = verifyOTP("user4", otp4);
    printf("First verification: %s\\n", result4a ? "SUCCESS" : "FAILED");
    bool result4b = verifyOTP("user4", otp4);
    printf("Second verification (reuse): %s\\n\\n", result4b ? "SUCCESS" : "FAILED");

    // Test Case 5: Non-existent user
    printf("Test Case 5: Non-existent User\\n");
    char otpBuffer5[OTP_LENGTH + 1];
    char* otp5 = authenticateAndGenerateOTP("nonexistent", "password", otpBuffer5);
    printf("OTP Generation for non-existent user: %s\\n\\n", otp5 == NULL ? "FAILED (User not found)" : "SUCCESS");

    return 0;
}
