#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define USERNAME_MAX_LEN 64
#define EMAIL_MAX_LEN 128
#define SALT_LEN 16
#define HASH_LEN 128 

// WARNING: This code uses placeholder functions for cryptography for demonstration.
// In a real-world application, ALWAYS use a vetted cryptographic library
// like OpenSSL or libsodium. DO NOT use this placeholder code in production.

// Securely clears memory. A volatile pointer is used to prevent the compiler
// from optimizing away the operation. For C11 and later with __STDC_LIB_EXT1__,
// memset_s is preferred.
void secure_zero_memory(void *v, size_t n) {
    volatile unsigned char *p = v;
    while (n--) {
        *p++ = 0;
    }
}

// Placeholder for a cryptographically secure random number generator.
// In production, use OpenSSL's RAND_bytes or read from /dev/urandom.
void placeholder_generate_salt(unsigned char* salt, size_t size) {
    // This is NOT secure and is for demonstration purposes only.
    for (size_t i = 0; i < size; ++i) {
        salt[i] = (unsigned char)(rand() % 256);
    }
}

// Placeholder for a strong key derivation function like PBKDF2 or Argon2.
// This function is INSECURE and for demonstration of logic ONLY.
void placeholder_hash_password(const char* password, const unsigned char* salt, unsigned char* hash_out, size_t hash_buf_size) {
    size_t pass_len = strlen(password);
    // Ensure we don't write past the buffer boundary
    size_t copy_len = (pass_len + SALT_LEN) < hash_buf_size ? pass_len : (hash_buf_size - SALT_LEN - 1);
    
    memset(hash_out, 0, hash_buf_size);
    memcpy(hash_out, password, copy_len);
    memcpy(hash_out + copy_len, salt, SALT_LEN);
}

// Performs a constant-time comparison of two byte arrays to prevent timing attacks.
bool constant_time_compare(const unsigned char* a, const unsigned char* b, size_t size) {
    unsigned char result = 0;
    for (size_t i = 0; i < size; ++i) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

typedef struct {
    char username[USERNAME_MAX_LEN];
    char email[EMAIL_MAX_LEN];
    unsigned char passwordHash[HASH_LEN];
    unsigned char salt[SALT_LEN];
} User;

typedef struct {
    User user;
    bool user_exists;
    bool is_logged_in;
} LoginSystem;

void system_init(LoginSystem* system) {
    if (!system) return;
    memset(system, 0, sizeof(LoginSystem));
    system->user_exists = false;
    system->is_logged_in = false;
}

bool register_user(LoginSystem* system, const char* username, const char* email, char* password) {
    if (!system || !username || !email || !password || strlen(password) == 0) return false;
    
    if (strlen(username) >= USERNAME_MAX_LEN || strlen(email) >= EMAIL_MAX_LEN) {
        printf("Registration failed: Input too long.\n");
        return false;
    }

    strncpy(system->user.username, username, USERNAME_MAX_LEN - 1);
    system->user.username[USERNAME_MAX_LEN - 1] = '\0';

    strncpy(system->user.email, email, EMAIL_MAX_LEN - 1);
    system->user.email[EMAIL_MAX_LEN - 1] = '\0';

    placeholder_generate_salt(system->user.salt, SALT_LEN);
    placeholder_hash_password(password, system->user.salt, system->user.passwordHash, HASH_LEN);
    
    secure_zero_memory(password, strlen(password));

    system->user_exists = true;
    system->is_logged_in = false;
    printf("User %s registered successfully.\n", system->user.username);
    return true;
}

bool login_user(LoginSystem* system, const char* username, char* password) {
    if (!system || !username || !password) return false;

    system->is_logged_in = false;
    if (!system->user_exists || strncmp(system->user.username, username, USERNAME_MAX_LEN) != 0) {
        printf("Login failed: User not found.\n");
        secure_zero_memory(password, strlen(password));
        return false;
    }

    unsigned char newHash[HASH_LEN];
    placeholder_hash_password(password, system->user.salt, newHash, HASH_LEN);
    secure_zero_memory(password, strlen(password));

    if (constant_time_compare(system->user.passwordHash, newHash, HASH_LEN)) {
        system->is_logged_in = true;
        printf("Login successful for user %s.\n", username);
        return true;
    }
    printf("Login failed: Incorrect password.\n");
    return false;
}

void logout_user(LoginSystem* system) {
    if (!system) return;
    system->is_logged_in = false;
    printf("User logged out.\n");
}

bool change_email(LoginSystem* system, const char* old_email, char* password, const char* new_email) {
    if (!system || !old_email || !password || !new_email) return false;

    if (!system->is_logged_in) {
        printf("Email change failed: User is not logged in.\n");
    } else if (strlen(new_email) >= EMAIL_MAX_LEN || strlen(new_email) == 0) {
        printf("Email change failed: Invalid new email.\n");
    } else if (strncmp(system->user.email, old_email, EMAIL_MAX_LEN) != 0) {
        printf("Email change failed: Old email does not match.\n");
    } else {
        unsigned char newHash[HASH_LEN];
        placeholder_hash_password(password, system->user.salt, newHash, HASH_LEN);
        if (!constant_time_compare(system->user.passwordHash, newHash, HASH_LEN)) {
            printf("Email change failed: Incorrect password.\n");
        } else {
            strncpy(system->user.email, new_email, EMAIL_MAX_LEN - 1);
            system->user.email[EMAIL_MAX_LEN - 1] = '\0';
            printf("Email for %s changed successfully to %s.\n", system->user.username, new_email);
            secure_zero_memory(password, strlen(password));
            return true;
        }
    }
    secure_zero_memory(password, strlen(password));
    return false;
}

int main() {
    // Seed for placeholder random generator. NOT for production use.
    srand(time(NULL));

    LoginSystem system;
    system_init(&system);

    // Passwords must be mutable char arrays to be cleared securely
    char password[] = "Password123!";
    register_user(&system, "testuser", "initial@example.com", password);

    printf("\n--- Test Case 1: Successful Email Change ---\n");
    char pass1[] = "Password123!";
    login_user(&system, "testuser", pass1);
    if (system.is_logged_in) {
        printf("Current email: %s\n", system.user.email);
        char pass_change[] = "Password123!";
        change_email(&system, "initial@example.com", pass_change, "new@example.com");
        printf("New email: %s\n", system.user.email);
    }
    logout_user(&system);

    printf("\n--- Test Case 2: Change Email with Wrong Password ---\n");
    char pass2[] = "Password123!";
    login_user(&system, "testuser", pass2);
    if (system.is_logged_in) {
        printf("Current email: %s\n", system.user.email);
        char wrong_pass[] = "WrongPassword!";
        change_email(&system, "new@example.com", wrong_pass, "another@example.com");
        printf("Email after failed attempt: %s\n", system.user.email);
    }
    logout_user(&system);

    printf("\n--- Test Case 3: Change Email with Wrong Old Email ---\n");
    char pass3[] = "Password123!";
    login_user(&system, "testuser", pass3);
    if (system.is_logged_in) {
        printf("Current email: %s\n", system.user.email);
        char pass_change3[] = "Password123!";
        change_email(&system, "wrong.old@example.com", pass_change3, "another@example.com");
        printf("Email after failed attempt: %s\n", system.user.email);
    }
    logout_user(&system);
    
    printf("\n--- Test Case 4: Change Email When Not Logged In ---\n");
    char pass4[] = "Password123!";
    change_email(&system, "new@example.com", pass4, "another@example.com");
    char pass4_login[] = "Password123!";
    login_user(&system, "testuser", pass4_login);
    if (system.is_logged_in) {
        printf("Email after failed attempt: %s\n", system.user.email);
    }
    logout_user(&system);

    printf("\n--- Test Case 5: Login with Wrong Password ---\n");
    char pass5[] = "NotThePassword";
    login_user(&system, "testuser", pass5);
    char pass5_change[] = "Password123!";
    change_email(&system, "new@example.com", pass5_change, "another@example.com");

    return 0;
}