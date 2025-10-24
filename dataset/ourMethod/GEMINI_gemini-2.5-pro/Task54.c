#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
#error "This code uses /dev/urandom and is not compatible with Windows. Please use a Windows-specific CSPRNG like CryptGenRandom."
#else
#include <unistd.h>
#include <fcntl.h>
#endif

#define OTP_LENGTH 6
#define MAX_INPUT_LEN 256
#define OTP_BUFFER_SIZE (OTP_LENGTH + 1)

// User structure
typedef struct {
    char username[MAX_INPUT_LEN];
    char password[MAX_INPUT_LEN];
} User;

// In a real application, passwords would be hashed and salted using a strong
// algorithm like Argon2. This is a simplified example focusing on OTP generation.
// Also, users would be stored in a secure database, not hardcoded.
User user_database[] = {
    {"alice", "password123"},
    {"bob", "securepass"}
};
const int num_users = sizeof(user_database) / sizeof(User);

// Helper to remove trailing newline from fgets input
void trim_newline(char* str) {
    if (str == NULL) return;
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

// Generates a cryptographically secure random OTP.
// Uses /dev/urandom on POSIX systems.
// Returns true on success, false on failure.
bool generate_otp(char* otp_buffer, size_t buffer_size) {
    if (otp_buffer == NULL || buffer_size < OTP_BUFFER_SIZE) {
        fprintf(stderr, "OTP buffer is NULL or too small.\n");
        return false;
    }

    int rand_fd = open("/dev/urandom", O_RDONLY);
    if (rand_fd < 0) {
        perror("Failed to open /dev/urandom");
        return false;
    }

    unsigned char random_bytes[OTP_LENGTH];
    ssize_t bytes_read = read(rand_fd, random_bytes, OTP_LENGTH);
    close(rand_fd);

    if (bytes_read < 0 || (size_t)bytes_read != OTP_LENGTH) {
        fprintf(stderr, "Failed to read enough random bytes.\n");
        return false;
    }

    for (int i = 0; i < OTP_LENGTH; i++) {
        // Modulo bias is negligible for this use case.
        otp_buffer[i] = (random_bytes[i] % 10) + '0';
    }
    otp_buffer[OTP_LENGTH] = '\0';

    // Clear sensitive data from stack
    memset(random_bytes, 0, sizeof(random_bytes));

    return true;
}


const User* find_user(const char* username) {
    if (username == NULL) return NULL;
    for (int i = 0; i < num_users; i++) {
        if (strncmp(username, user_database[i].username, MAX_INPUT_LEN) == 0) {
            return &user_database[i];
        }
    }
    return NULL;
}

bool perform_2fa() {
    char username[MAX_INPUT_LEN] = {0};
    char password[MAX_INPUT_LEN] = {0};
    char entered_otp[MAX_INPUT_LEN] = {0};

    printf("Enter username: ");
    if (fgets(username, sizeof(username), stdin) == NULL) return false;
    trim_newline(username);

    printf("Enter password: ");
    if (fgets(password, sizeof(password), stdin) == NULL) return false;
    trim_newline(password);

    const User* user = find_user(username);

    if (user != NULL && strncmp(user->password, password, MAX_INPUT_LEN) == 0) {
        printf("Password correct. 2FA required.\n");
        
        char otp[OTP_BUFFER_SIZE];
        if (!generate_otp(otp, sizeof(otp))) {
            fprintf(stderr, "Failed to generate OTP. Aborting.\n");
            return false;
        }

        // In a real system, this OTP would be sent via SMS, email, or an authenticator app.
        printf("Generated OTP (for simulation): %s\n", otp);

        printf("Enter the %d-digit OTP: ", OTP_LENGTH);
        if (fgets(entered_otp, sizeof(entered_otp), stdin) == NULL) {
            memset(otp, 0, sizeof(otp)); // Clear before returning
            return false;
        }
        trim_newline(entered_otp);

        bool success = (strncmp(entered_otp, otp, OTP_BUFFER_SIZE) == 0);

        // Securely clear sensitive data
        memset(otp, 0, sizeof(otp));
        memset(entered_otp, 0, sizeof(entered_otp));

        if (success) {
            printf("Login successful!\n");
            return true;
        } else {
            printf("Invalid OTP. Login failed.\n");
            return false;
        }

    } else {
        printf("Invalid username or password.\n");
        return false;
    }
}

void run_test_case(int case_num) {
    printf("\n--- Test Case %d ---\n", case_num);
    switch(case_num) {
        case 1:
            printf("Goal: Successful login.\n");
            printf("To pass: Enter username 'alice' and password 'password123'.\n");
            printf("Then enter the generated OTP correctly.\n");
            break;
        case 2:
            printf("Goal: Fail login with incorrect password.\n");
            printf("To pass: Enter username 'alice' and password 'wrongpassword'.\n");
            break;
        case 3:
            printf("Goal: Fail login with non-existent user.\n");
            printf("To pass: Enter username 'charlie' and any password.\n");
            break;
        case 4:
            printf("Goal: Fail login with incorrect OTP.\n");
            printf("To pass: Enter username 'bob' and password 'securepass'.\n");
            printf("Then enter an incorrect OTP (e.g., '000000').\n");
            break;
        case 5:
            printf("Goal: Successful login again to show OTP is random.\n");
            printf("To pass: Enter username 'alice' and password 'password123' again.\n");
            printf("Observe that the OTP is different from Test Case 1.\n");
            break;
    }
    perform_2fa();
}

int main() {
    printf("2FA Login Simulation (C Version)\n");
    printf("================================\n");

    run_test_case(1);
    run_test_case(2);
    run_test_case(3);
    run_test_case(4);
    run_test_case(5);
    
    printf("\n--- All test cases completed. ---\n");

    return 0;
}