#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define OTP_LEN 7 // 6 digits + null terminator

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
} User;

typedef struct {
    char username[MAX_USERNAME_LEN];
    char otp[OTP_LEN];
} UserOTP;

User users[MAX_USERS];
int user_count = 0;

UserOTP user_otps[MAX_USERS];
int otp_count = 0;

/**
 * Initializes the user database.
 */
void initialize_users() {
    strcpy(users[0].username, "user1");
    strcpy(users[0].password, "pass123");
    strcpy(users[1].username, "user2");
    strcpy(users[1].password, "secret456");
    strcpy(users[2].username, "user3");
    strcpy(users[2].password, "qwerty");
    user_count = 3;
}

/**
 * Verifies the user's primary credentials.
 * @return 1 if valid, 0 otherwise.
 */
int verifyUser(const char* username, const char* password) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * Generates a random 6-digit OTP, stores it, and returns a pointer to it.
 * NOTE: Returns a pointer to a static buffer, which is overwritten on each call.
 * @return A pointer to the OTP string, or NULL if user does not exist.
 */
const char* generateAndSendOtp(const char* username) {
    int user_exists = 0;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            user_exists = 1;
            break;
        }
    }
    if (!user_exists) return NULL;

    // Generate OTP
    int otp_value = 100000 + (rand() % 900000);
    static char otp_buffer[OTP_LEN];
    sprintf(otp_buffer, "%d", otp_value);

    // Store OTP
    int otp_index = -1;
    for (int i = 0; i < otp_count; i++) {
        if (strcmp(user_otps[i].username, username) == 0) {
            otp_index = i;
            break;
        }
    }
    if (otp_index == -1 && otp_count < MAX_USERS) {
        otp_index = otp_count++;
        strcpy(user_otps[otp_index].username, username);
    }
    if (otp_index != -1) {
        strcpy(user_otps[otp_index].otp, otp_buffer);
        printf("OTP sent for user '%s'. OTP is: %s (for simulation purposes)\n", username, otp_buffer);
        return otp_buffer;
    }

    return NULL;
}

/**
 * Verifies the entered OTP against the stored OTP for a user.
 * @return 1 if the OTP is correct, 0 otherwise.
 */
int verifyOtp(const char* username, const char* enteredOtp) {
    for (int i = 0; i < otp_count; i++) {
        if (strcmp(user_otps[i].username, username) == 0 && strcmp(user_otps[i].otp, enteredOtp) == 0) {
            // OTP is single-use, invalidate it by clearing the username.
            // A more robust implementation would shift the array elements.
            strcpy(user_otps[i].username, ""); 
            return 1;
        }
    }
    return 0;
}


int main() {
    srand(time(NULL)); // Seed the random number generator
    initialize_users();

    // --- Test Case 1: Successful Login ---
    printf("--- Test Case 1: Successful Login ---\n");
    const char* user1 = "user1";
    const char* pass1 = "pass123";
    if (verifyUser(user1, pass1)) {
        printf("Primary authentication successful for %s\n", user1);
        const char* otp1 = generateAndSendOtp(user1);
        // Simulate user entering the correct OTP
        if (verifyOtp(user1, otp1)) {
            printf("2FA successful. Login complete for %s\n", user1);
        } else {
            printf("2FA failed. Invalid OTP.\n");
        }
    } else {
        printf("Primary authentication failed for %s\n", user1);
    }
    printf("\n----------------------------------------\n\n");

    // --- Test Case 2: Incorrect Password ---
    printf("--- Test Case 2: Incorrect Password ---\n");
    const char* user2 = "user2";
    const char* pass2_wrong = "wrongpassword";
    if (verifyUser(user2, pass2_wrong)) {
        printf("Primary authentication successful for %s\n", user2);
        generateAndSendOtp(user2);
    } else {
        printf("Primary authentication failed for %s. Incorrect username or password.\n", user2);
    }
    printf("\n----------------------------------------\n\n");

    // --- Test Case 3: Incorrect Username ---
    printf("--- Test Case 3: Incorrect Username ---\n");
    const char* user3_wrong = "nonexistentuser";
    const char* pass3 = "qwerty";
    if (verifyUser(user3_wrong, pass3)) {
        printf("Primary authentication successful for %s\n", user3_wrong);
        generateAndSendOtp(user3_wrong);
    } else {
        printf("Primary authentication failed for %s. Incorrect username or password.\n", user3_wrong);
    }
    printf("\n----------------------------------------\n\n");

    // --- Test Case 4: Correct Password, Incorrect OTP ---
    printf("--- Test Case 4: Correct Password, Incorrect OTP ---\n");
    const char* user4 = "user3";
    const char* pass4 = "qwerty";
    if (verifyUser(user4, pass4)) {
        printf("Primary authentication successful for %s\n", user4);
        generateAndSendOtp(user4);
        // Simulate user entering a wrong OTP
        const char* wrongOtp = "000000";
        printf("User '%s' enters OTP: %s\n", user4, wrongOtp);
        if (verifyOtp(user4, wrongOtp)) {
            printf("2FA successful. Login complete for %s\n", user4);
        } else {
            printf("2FA failed. Invalid OTP for %s\n", user4);
        }
    } else {
        printf("Primary authentication failed for %s\n", user4);
    }
    printf("\n----------------------------------------\n\n");

    // --- Test Case 5: Another Successful Login ---
    printf("--- Test Case 5: Another Successful Login ---\n");
    const char* user5 = "user2";
    const char* pass5 = "secret456";
    if (verifyUser(user5, pass5)) {
        printf("Primary authentication successful for %s\n", user5);
        const char* otp5 = generateAndSendOtp(user5);
        // Simulate user entering the correct OTP
        if (verifyOtp(user5, otp5)) {
            printf("2FA successful. Login complete for %s\n", user5);
        } else {
            printf("2FA failed. Invalid OTP.\n");
        }
    } else {
        printf("Primary authentication failed for %s\n", user5);
    }
    printf("\n----------------------------------------\n\n");

    return 0;
}