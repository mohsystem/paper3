#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_USERS 5
#define USERNAME_LEN 32
#define PASSWORD_LEN 32
#define OTP_LEN 7 // 6 digits + null terminator

// User database structure
typedef struct {
    char username[USERNAME_LEN];
    char password[PASSWORD_LEN];
} User;

// OTP storage structure
typedef struct {
    char username[USERNAME_LEN];
    char otp[OTP_LEN];
} OtpEntry;

// In-memory storage for user credentials and OTPs
// WARNING: Storing plain-text passwords is insecure. Use hashed passwords in production.
User user_database[MAX_USERS];
int user_count = 0;

OtpEntry otp_storage[MAX_USERS];
int otp_count = 0;

void initialize_database() {
    if (user_count == 0) {
        strcpy(user_database[0].username, "alice");
        strcpy(user_database[0].password, "password123");
        strcpy(user_database[1].username, "bob");
        strcpy(user_database[1].password, "bob@1234");
        user_count = 2;
    }
}

/**
 * @brief Factor 1: Validates username and password.
 *
 * @param username The user's username.
 * @param password The user's password.
 * @return 1 if credentials are valid, 0 otherwise.
 */
int login(const char* username, const char* password) {
    if (username == NULL || password == NULL) return 0;
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0 &&
            strcmp(user_database[i].password, password) == 0) {
            return 1;
        }
    }
    return 0;
}

/**
 * @brief Generates and "sends" a 6-digit OTP for the user.
 *
 * @param username The user to generate an OTP for.
 * @param out_otp Buffer to store the generated OTP.
 * @param buffer_size Size of the out_otp buffer.
 * @return 1 on success, 0 on failure (e.g., user not found).
 */
int generate_and_send_otp(const char* username, char* out_otp, size_t buffer_size) {
    int user_found = 0;
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(user_database[i].username, username) == 0) {
            user_found = 1;
            break;
        }
    }
    if (!user_found) return 0;

    // WARNING: rand() is not cryptographically secure. For a secure implementation,
    // use platform-specific APIs like /dev/urandom on Linux/macOS or BCryptGenRandom on Windows.
    // This is a simplified example.
    int otp_value = 100000 + (rand() % 900000);
    snprintf(out_otp, buffer_size, "%06d", otp_value);

    // Store the OTP
    int stored_index = -1;
    for (int i = 0; i < otp_count; ++i) {
        if (strcmp(otp_storage[i].username, username) == 0) {
            stored_index = i;
            break;
        }
    }
    if (stored_index == -1 && otp_count < MAX_USERS) {
        stored_index = otp_count++;
        strcpy(otp_storage[stored_index].username, username);
    }
    if(stored_index != -1) {
       strcpy(otp_storage[stored_index].otp, out_otp);
       printf("OTP sent to %s. Your OTP is: %s\n", username, out_otp);
       return 1;
    }
    return 0; // OTP storage is full
}

/**
 * @brief Factor 2: Validates the provided OTP for the user.
 *
 * @param username The user's username.
 * @param otp The one-time password provided by the user.
 * @return 1 if the OTP is valid, 0 otherwise.
 */
int validate_otp(const char* username, const char* otp) {
    if (username == NULL || otp == NULL) return 0;
    for (int i = 0; i < otp_count; ++i) {
        if (strcmp(otp_storage[i].username, username) == 0 &&
            strcmp(otp_storage[i].otp, otp) == 0) {
            
            // Invalidate the OTP by removing the entry
            // (shifting the last element into its place)
            otp_storage[i] = otp_storage[otp_count - 1];
            otp_count--;
            
            return 1;
        }
    }
    return 0;
}

int main() {
    // Seed the insecure random number generator
    srand((unsigned int)time(NULL));
    initialize_database();

    printf("--- Running 2FA Login Test Cases ---\n");
    char generated_otp[OTP_LEN];

    // Test Case 1: Successful Login
    printf("\n--- Test Case 1: Successful Login ---\n");
    const char* user1 = "alice";
    const char* pass1 = "password123";
    printf("Attempting to log in user: %s\n", user1);
    if (login(user1, pass1)) {
        printf("Step 1 (Password) successful.\n");
        if(generate_and_send_otp(user1, generated_otp, sizeof(generated_otp))) {
            if (validate_otp(user1, generated_otp)) {
                printf("Step 2 (OTP) successful. Login successful for %s!\n", user1);
            } else {
                printf("Step 2 (OTP) failed. Login failed.\n");
            }
        }
    } else {
        printf("Step 1 (Password) failed. Invalid username or password.\n");
    }

    // Test Case 2: Invalid Password
    printf("\n--- Test Case 2: Invalid Password ---\n");
    const char* user2 = "bob";
    const char* pass2 = "wrongpassword";
    printf("Attempting to log in user: %s\n", user2);
    if (login(user2, pass2)) {
        printf("Step 1 (Password) successful.\n");
    } else {
        printf("Step 1 (Password) failed. Invalid username or password.\n");
    }

    // Test Case 3: Invalid Username
    printf("\n--- Test Case 3: Invalid Username ---\n");
    const char* user3 = "charlie";
    const char* pass3 = "password123";
    printf("Attempting to log in user: %s\n", user3);
    if (login(user3, pass3)) {
        printf("Step 1 (Password) successful.\n");
    } else {
        printf("Step 1 (Password) failed. Invalid username or password.\n");
    }

    // Test Case 4: Incorrect OTP
    printf("\n--- Test Case 4: Incorrect OTP ---\n");
    const char* user4 = "bob";
    const char* pass4 = "bob@1234";
    printf("Attempting to log in user: %s\n", user4);
    if (login(user4, pass4)) {
        printf("Step 1 (Password) successful.\n");
        if(generate_and_send_otp(user4, generated_otp, sizeof(generated_otp))) {
            const char* incorrect_otp = "111111";
            printf("User provides OTP: %s\n", incorrect_otp);
            if (validate_otp(user4, incorrect_otp)) {
                printf("Step 2 (OTP) successful. Login successful for %s!\n", user4);
            } else {
                printf("Step 2 (OTP) failed. Login failed.\n");
            }
        }
    } else {
        printf("Step 1 (Password) failed. Invalid username or password.\n");
    }

    // Test Case 5: Another Successful Login
    printf("\n--- Test Case 5: Another Successful Login ---\n");
    const char* user5 = "bob";
    const char* pass5 = "bob@1234";
    printf("Attempting to log in user: %s\n", user5);
    if (login(user5, pass5)) {
        printf("Step 1 (Password) successful.\n");
        if (generate_and_send_otp(user5, generated_otp, sizeof(generated_otp))) {
            if (validate_otp(user5, generated_otp)) {
                printf("Step 2 (OTP) successful. Login successful for %s!\n", user5);
            } else {
                printf("Step 2 (OTP) failed. Login failed.\n");
            }
        }
    } else {
        printf("Step 1 (Password) failed. Invalid username or password.\n");
    }

    return 0;
}