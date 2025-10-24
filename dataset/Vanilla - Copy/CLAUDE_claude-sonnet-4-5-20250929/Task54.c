
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define MAX_USERS 100
#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_EMAIL 100
#define OTP_LENGTH 6
#define OTP_EXPIRY_MINUTES 5

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    char email[MAX_EMAIL];
} User;

typedef struct {
    char code[OTP_LENGTH + 1];
    time_t generatedAt;
} OTP;

typedef struct {
    User users[MAX_USERS];
    int userCount;
    char otpUsernames[MAX_USERS][MAX_USERNAME];
    OTP otps[MAX_USERS];
    int otpCount;
} AuthSystem;

void initAuthSystem(AuthSystem* system) {
    system->userCount = 0;
    system->otpCount = 0;
}

void generateOTP(char* otp) {
    for (int i = 0; i < OTP_LENGTH; i++) {
        otp[i] = '0' + (rand() % 10);
    }
    otp[OTP_LENGTH] = '\\0';
}

bool registerUser(AuthSystem* system, const char* username, const char* password, const char* email) {
    for (int i = 0; i < system->userCount; i++) {
        if (strcmp(system->users[i].username, username) == 0) {
            return false;
        }
    }
    
    if (system->userCount >= MAX_USERS) {
        return false;
    }
    
    strcpy(system->users[system->userCount].username, username);
    strcpy(system->users[system->userCount].password, password);
    strcpy(system->users[system->userCount].email, email);
    system->userCount++;
    return true;
}

bool authenticateCredentials(AuthSystem* system, const char* username, const char* password, char* otp) {
    int userIndex = -1;
    for (int i = 0; i < system->userCount; i++) {
        if (strcmp(system->users[i].username, username) == 0) {
            userIndex = i;
            break;
        }
    }
    
    if (userIndex == -1 || strcmp(system->users[userIndex].password, password) != 0) {
        return false;
    }
    
    generateOTP(otp);
    
    int otpIndex = -1;
    for (int i = 0; i < system->otpCount; i++) {
        if (strcmp(system->otpUsernames[i], username) == 0) {
            otpIndex = i;
            break;
        }
    }
    
    if (otpIndex == -1) {
        otpIndex = system->otpCount;
        strcpy(system->otpUsernames[otpIndex], username);
        system->otpCount++;
    }
    
    strcpy(system->otps[otpIndex].code, otp);
    system->otps[otpIndex].generatedAt = time(NULL);
    
    return true;
}

bool verifyOTP(AuthSystem* system, const char* username, const char* enteredOTP) {
    int otpIndex = -1;
    for (int i = 0; i < system->otpCount; i++) {
        if (strcmp(system->otpUsernames[i], username) == 0) {
            otpIndex = i;
            break;
        }
    }
    
    if (otpIndex == -1) {
        return false;
    }
    
    time_t now = time(NULL);
    double elapsed = difftime(now, system->otps[otpIndex].generatedAt) / 60.0;
    
    if (elapsed >= OTP_EXPIRY_MINUTES) {
        return false;
    }
    
    return strcmp(system->otps[otpIndex].code, enteredOTP) == 0;
}

const char* performLogin(AuthSystem* system, const char* username, const char* password, const char* enteredOTP) {
    char generatedOTP[OTP_LENGTH + 1];
    
    if (!authenticateCredentials(system, username, password, generatedOTP)) {
        return "Login Failed: Invalid credentials";
    }
    
    if (verifyOTP(system, username, enteredOTP)) {
        return "Login Successful";
    } else {
        return "Login Failed: Invalid or expired OTP";
    }
}

int main() {
    srand(time(NULL));
    AuthSystem authSystem;
    initAuthSystem(&authSystem);
    
    printf("=== Two-Factor Authentication System ===\\n\\n");
    
    // Test Case 1: Successful registration and login
    printf("Test Case 1: Successful registration and login\\n");
    registerUser(&authSystem, "user1", "pass123", "user1@example.com");
    char otp1[OTP_LENGTH + 1];
    authenticateCredentials(&authSystem, "user1", "pass123", otp1);
    printf("Generated OTP: %s\\n", otp1);
    const char* result1 = performLogin(&authSystem, "user1", "pass123", otp1);
    printf("Result: %s\\n\\n", result1);
    
    // Test Case 2: Wrong password
    printf("Test Case 2: Wrong password\\n");
    registerUser(&authSystem, "user2", "pass456", "user2@example.com");
    const char* result2 = performLogin(&authSystem, "user2", "wrongpass", "123456");
    printf("Result: %s\\n\\n", result2);
    
    // Test Case 3: Wrong OTP
    printf("Test Case 3: Wrong OTP\\n");
    registerUser(&authSystem, "user3", "pass789", "user3@example.com");
    char otp3[OTP_LENGTH + 1];
    authenticateCredentials(&authSystem, "user3", "pass789", otp3);
    printf("Generated OTP: %s\\n", otp3);
    const char* result3 = performLogin(&authSystem, "user3", "pass789", "000000");
    printf("Result: %s\\n\\n", result3);
    
    // Test Case 4: User doesn't exist\n    printf("Test Case 4: User doesn't exist\
");\n    const char* result4 = performLogin(&authSystem, "nonexistent", "pass000", "123456");\n    printf("Result: %s\
\
", result4);\n    \n    // Test Case 5: Multiple users with successful login\n    printf("Test Case 5: Multiple users with successful login\
");\n    registerUser(&authSystem, "user4", "secure123", "user4@example.com");\n    registerUser(&authSystem, "user5", "secure456", "user5@example.com");\n    char otp4[OTP_LENGTH + 1], otp5[OTP_LENGTH + 1];\n    authenticateCredentials(&authSystem, "user4", "secure123", otp4);\n    authenticateCredentials(&authSystem, "user5", "secure456", otp5);\n    printf("User4 OTP: %s\
", otp4);\n    printf("User5 OTP: %s\
", otp5);\n    const char* result5a = performLogin(&authSystem, "user4", "secure123", otp4);\n    const char* result5b = performLogin(&authSystem, "user5", "secure456", otp5);\n    printf("User4 Result: %s\
", result5a);\n    printf("User5 Result: %s\
", result5b);
    
    return 0;
}
