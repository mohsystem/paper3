
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>

#define OTP_LENGTH 6
#define OTP_VALIDITY_SECONDS 300
#define MAX_USERS 100
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 65
#define HASH_HEX_LEN 65

typedef struct {
    char hashedOTP[HASH_HEX_LEN];
    long long expiryTime;
    int attempts;
    int active;
} OTPData;

typedef struct {
    char username[MAX_USERNAME_LEN];
    char hashedPassword[HASH_HEX_LEN];
    int active;
} UserCredential;

typedef struct {
    char username[MAX_USERNAME_LEN];
    OTPData otpData;
    int active;
} OTPStorage;

UserCredential userCredentials[MAX_USERS];
OTPStorage otpStorage[MAX_USERS];
int userCount = 0;
int otpCount = 0;

void generateOTP(char* otp) {
    srand(time(NULL) + rand());
    for (int i = 0; i < OTP_LENGTH; i++) {
        otp[i] = '0' + (rand() % 10);
    }
    otp[OTP_LENGTH] = '\\0';
}

void hashPassword(const char* password, char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
    output[HASH_HEX_LEN - 1] = '\\0';
}

int registerUser(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0 || 
        password == NULL || strlen(password) < 8) {
        return 0;
    }
    
    if (userCount >= MAX_USERS) {
        return 0;
    }
    
    for (int i = 0; i < userCount; i++) {
        if (userCredentials[i].active && 
            strcmp(userCredentials[i].username, username) == 0) {
            return 0;
        }
    }
    
    strncpy(userCredentials[userCount].username, username, MAX_USERNAME_LEN - 1);
    userCredentials[userCount].username[MAX_USERNAME_LEN - 1] = '\\0';
    hashPassword(password, userCredentials[userCount].hashedPassword);
    userCredentials[userCount].active = 1;
    userCount++;
    
    return 1;
}

int authenticateUser(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return 0;
    }
    
    char inputHash[HASH_HEX_LEN];
    hashPassword(password, inputHash);
    
    for (int i = 0; i < userCount; i++) {
        if (userCredentials[i].active && 
            strcmp(userCredentials[i].username, username) == 0) {
            return strcmp(userCredentials[i].hashedPassword, inputHash) == 0;
        }
    }
    
    return 0;
}

int sendOTP(const char* username, char* otp) {
    int userExists = 0;
    for (int i = 0; i < userCount; i++) {
        if (userCredentials[i].active && 
            strcmp(userCredentials[i].username, username) == 0) {
            userExists = 1;
            break;
        }
    }
    
    if (!userExists) {
        return 0;
    }
    
    generateOTP(otp);
    char hashedOTP[HASH_HEX_LEN];
    hashPassword(otp, hashedOTP);
    
    long long expiryTime = (long long)time(NULL) + OTP_VALIDITY_SECONDS;
    
    int index = -1;
    for (int i = 0; i < otpCount; i++) {
        if (otpStorage[i].active && 
            strcmp(otpStorage[i].username, username) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        if (otpCount >= MAX_USERS) {
            return 0;
        }
        index = otpCount++;
        strncpy(otpStorage[index].username, username, MAX_USERNAME_LEN - 1);
        otpStorage[index].username[MAX_USERNAME_LEN - 1] = '\\0';
    }
    
    strcpy(otpStorage[index].otpData.hashedOTP, hashedOTP);
    otpStorage[index].otpData.expiryTime = expiryTime;
    otpStorage[index].otpData.attempts = 0;
    otpStorage[index].active = 1;
    
    return 1;
}

int verifyOTP(const char* username, const char* otp) {
    if (username == NULL || otp == NULL) {
        return 0;
    }
    
    int index = -1;
    for (int i = 0; i < otpCount; i++) {
        if (otpStorage[i].active && 
            strcmp(otpStorage[i].username, username) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        return 0;
    }
    
    OTPData* otpData = &otpStorage[index].otpData;
    
    if (otpData->attempts >= 3) {
        otpStorage[index].active = 0;
        return 0;
    }
    
    otpData->attempts++;
    
    long long currentTime = (long long)time(NULL);
    if (currentTime > otpData->expiryTime) {
        otpStorage[index].active = 0;
        return 0;
    }
    
    char hashedInputOTP[HASH_HEX_LEN];
    hashPassword(otp, hashedInputOTP);
    
    int isValid = (strcmp(otpData->hashedOTP, hashedInputOTP) == 0);
    
    if (isValid) {
        otpStorage[index].active = 0;
    }
    
    return isValid;
}

int twoFactorLogin(const char* username, const char* password, const char* otp) {
    if (!authenticateUser(username, password)) {
        return 0;
    }
    return verifyOTP(username, otp);
}

int main() {
    printf("Two-Factor Authentication System Test Cases\\n\\n");
    
    // Test Case 1: Successful 2FA login
    printf("Test Case 1: Successful 2FA login\\n");
    registerUser("user1", "SecurePass123");
    char otp1[OTP_LENGTH + 1];
    sendOTP("user1", otp1);
    int result1 = twoFactorLogin("user1", "SecurePass123", otp1);
    printf("Result: %s\\n\\n", result1 ? "SUCCESS" : "FAILED");
    
    // Test Case 2: Wrong password
    printf("Test Case 2: Wrong password\\n");
    registerUser("user2", "SecurePass456");
    char otp2[OTP_LENGTH + 1];
    sendOTP("user2", otp2);
    int result2 = twoFactorLogin("user2", "WrongPassword", otp2);
    printf("Result: %s\\n\\n", result2 ? "SUCCESS" : "FAILED (Expected)");
    
    // Test Case 3: Wrong OTP
    printf("Test Case 3: Wrong OTP\\n");
    registerUser("user3", "SecurePass789");
    char otp3[OTP_LENGTH + 1];
    sendOTP("user3", otp3);
    int result3 = twoFactorLogin("user3", "SecurePass789", "000000");
    printf("Result: %s\\n\\n", result3 ? "SUCCESS" : "FAILED (Expected)");
    
    // Test Case 4: Valid OTP verification
    printf("Test Case 4: Valid OTP verification\\n");
    registerUser("user4", "SecurePass101");
    char otp4[OTP_LENGTH + 1];
    sendOTP("user4", otp4);
    int result4 = verifyOTP("user4", otp4);
    printf("Result: %s\\n\\n", result4 ? "SUCCESS" : "FAILED");
    
    // Test Case 5: Multiple users
    printf("Test Case 5: Multiple users handling\\n");
    registerUser("user5", "SecurePass202");
    registerUser("user6", "SecurePass303");
    char otp5[OTP_LENGTH + 1], otp6[OTP_LENGTH + 1];
    sendOTP("user5", otp5);
    sendOTP("user6", otp6);
    int result5a = twoFactorLogin("user5", "SecurePass202", otp5);
    int result5b = twoFactorLogin("user6", "SecurePass303", otp6);
    printf("User5 Result: %s\\n", result5a ? "SUCCESS" : "FAILED");
    printf("User6 Result: %s\\n", result5b ? "SUCCESS" : "FAILED");
    
    return 0;
}
