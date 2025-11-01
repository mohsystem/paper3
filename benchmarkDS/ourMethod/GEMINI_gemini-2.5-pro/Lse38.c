#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

#define SALT_LENGTH 16
#define ITERATIONS 310000 // OWASP recommendation as of 2023 for PBKDF2-HMAC-SHA256
#define KEY_LENGTH 32     // 256 bits
#define MAX_USERNAME_LEN 20
#define MIN_USERNAME_LEN 5
#define MAX_PASSWORD_LEN 100
#define MIN_PASSWORD_LEN 8
#define MAX_USERS 10

// In a real application, this would be a database.
typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    unsigned char salt[SALT_LENGTH];
    unsigned char hash[KEY_LENGTH];
} UserData;

UserData user_database[MAX_USERS];
int user_count = 0;

// Utility to handle OpenSSL errors
void handle_openssl_errors() {
    // In a real application, log errors securely.
    // For this example, we print to stderr and exit.
    // ERR_print_errors_fp(stderr);
    fprintf(stderr, "An OpenSSL error occurred. Aborting.\n");
    exit(EXIT_FAILURE);
}

// Securely clear sensitive data from memory.
void secure_clear_memory(void *ptr, size_t len) {
    #if defined(_WIN32)
        SecureZeroMemory(ptr, len);
    #elif defined(HAVE_MEMSET_S)
        if (memset_s(ptr, len, 0, len) != 0) {
            // Handle error, maybe log it
        }
    #else
        // Fallback to a volatile pointer to prevent optimization
        volatile unsigned char *p = ptr;
        for (size_t i = 0; i < len; ++i) {
            p[i] = 0;
        }
    #endif
}

// Function to validate password complexity
bool is_password_valid(const char *password) {
    if (!password || strlen(password) < MIN_PASSWORD_LEN) return false;
    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;
    for (int i = 0; password[i] != '\0'; i++) {
        if (isupper(password[i])) has_upper = true;
        else if (islower(password[i])) has_lower = true;
        else if (isdigit(password[i])) has_digit = true;
        else if (strchr("@$!%*?&", password[i])) has_special = true;
    }
    return has_upper && has_lower && has_digit && has_special;
}

// Function to validate username format
bool is_username_valid(const char *username) {
    if (!username) return false;
    size_t len = strlen(username);
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) return false;
    for (int i = 0; i < len; i++) {
        if (!isalnum(username[i])) return false;
    }
    return true;
}


bool hash_password(const char *password, const unsigned char *salt, unsigned char *hash_out) {
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_LENGTH,
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        hash_out
    );
    return result == 1;
}

bool register_user(const char *username, char *password) {
    if (user_count >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return false;
    }

    if (!is_username_valid(username) || !is_password_valid(password)) {
        printf("Registration failed: Username or password does not meet complexity requirements.\n");
        return false;
    }

    for (int i = 0; i < user_count; i++) {
        if (strncmp(user_database[i].username, username, MAX_USERNAME_LEN) == 0) {
            printf("Registration failed: Username '%s' already exists.\n", username);
            return false;
        }
    }
    
    strncpy(user_database[user_count].username, username, MAX_USERNAME_LEN);
    user_database[user_count].username[MAX_USERNAME_LEN] = '\0';

    if (RAND_bytes(user_database[user_count].salt, SALT_LENGTH) != 1) {
        handle_openssl_errors();
    }

    if (!hash_password(password, user_database[user_count].salt, user_database[user_count].hash)) {
        handle_openssl_errors();
    }

    secure_clear_memory(password, strlen(password));
    user_count++;
    printf("User '%s' registered successfully.\n", username);
    return true;
}

bool login(const char *username, char *password) {
    if (username == NULL || password == NULL || username[0] == '\0' || password[0] == '\0') {
        printf("Login failed: Username or password cannot be empty.\n");
        return false;
    }

    UserData *user = NULL;
    for (int i = 0; i < user_count; i++) {
        if (strncmp(user_database[i].username, username, MAX_USERNAME_LEN) == 0) {
            user = &user_database[i];
            break;
        }
    }

    if (user == NULL) {
        printf("Login failed: Incorrect username or password.\n"); // Generic message
        return false;
    }

    unsigned char attempted_hash[KEY_LENGTH];
    if (!hash_password(password, user->salt, attempted_hash)) {
        handle_openssl_errors();
    }
    secure_clear_memory(password, strlen(password));

    // Constant-time comparison to prevent timing attacks
    if (CRYPTO_memcmp(attempted_hash, user->hash, KEY_LENGTH) == 0) {
        printf("Login successful for user '%s'. Redirecting to dashboard.\n", username);
        return true;
    } else {
        printf("Login failed: Incorrect username or password.\n");
        return false;
    }
}

int main() {
    char pass_buffer[MAX_PASSWORD_LEN + 1];

    printf("--- Setting up user ---\n");
    strncpy(pass_buffer, "SecureC_Pass#1", MAX_PASSWORD_LEN);
    register_user("cuser1", pass_buffer);
    printf("\n--- Running Test Cases ---\n");

    // Test Case 1: Correct username and password
    printf("1. Testing correct credentials...\n");
    strncpy(pass_buffer, "SecureC_Pass#1", MAX_PASSWORD_LEN);
    login("cuser1", pass_buffer);

    // Test Case 2: Correct username, incorrect password
    printf("\n2. Testing incorrect password...\n");
    strncpy(pass_buffer, "WrongPassword!1", MAX_PASSWORD_LEN);
    login("cuser1", pass_buffer);

    // Test Case 3: Incorrect username
    printf("\n3. Testing incorrect username...\n");
    strncpy(pass_buffer, "SecureC_Pass#1", MAX_PASSWORD_LEN);
    login("nonexistentuser", pass_buffer);
    
    // Test Case 4: Empty password
    printf("\n4. Testing empty password...\n");
    strncpy(pass_buffer, "", MAX_PASSWORD_LEN);
    login("cuser1", pass_buffer);

    // Test Case 5: Registration with weak password
    printf("\n5. Testing registration with weak password...\n");
    strncpy(pass_buffer, "pass", MAX_PASSWORD_LEN);
    register_user("weakuserc", pass_buffer);

    return 0;
}