#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// This code requires OpenSSL.
// Compile with: gcc your_file.c -o your_app -lssl -lcrypto
#include <openssl/rand.h>
#include <openssl/evp.h>

#define HASH_ITERATIONS 210000
#define SALT_SIZE 16
#define HASH_KEY_LENGTH 32 // 256 bits

typedef struct User {
    char* username;
    char* salt; // hex encoded
    char* hashed_password; // hex encoded
    char* first_name;
    char* last_name;
    char* email;
    struct User* next;
} User;

// In-memory linked list to simulate a database
static User* user_database = NULL;

// Securely wipe memory. A volatile pointer is used to prevent the compiler
// from optimizing away this operation.
void secure_wipe(void* data, size_t len) {
    volatile unsigned char* p = (volatile unsigned char*)data;
    while (len--) {
        *p++ = 0;
    }
}

// Helper to convert byte array to a hex string.
// The caller must free the returned string.
char* bytes_to_hex(const unsigned char* bytes, size_t len) {
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) return NULL;
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_str + i * 2, "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

void free_user_list() {
    User* current = user_database;
    while (current != NULL) {
        User* next = current->next;
        free(current->username);
        free(current->salt);
        free(current->hashed_password);
        free(current->first_name);
        free(current->last_name);
        free(current->email);
        free(current);
        current = next;
    }
    user_database = NULL;
}

// Simplified validation functions
int is_valid_username(const char* username) {
    size_t len = strlen(username);
    if (len < 3 || len > 20) return 0;
    for (size_t i = 0; i < len; ++i) {
        if (!isalnum(username[i]) && username[i] != '_') return 0;
    }
    return 1;
}

int is_strong_password(const char* password) {
    size_t len = strlen(password);
    if (len < 12) return 0;
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    const char* specials = "@$!%*?&";
    for (size_t i = 0; i < len; ++i) {
        if (isupper(password[i])) has_upper = 1;
        else if (islower(password[i])) has_lower = 1;
        else if (isdigit(password[i])) has_digit = 1;
        else if (strchr(specials, password[i])) has_special = 1;
    }
    return has_upper && has_lower && has_digit && has_special;
}

int is_valid_email(const char* email) {
    const char* at = strchr(email, '@');
    if (!at || at == email) return 0;
    const char* dot = strrchr(at, '.');
    if (!dot || dot == at + 1 || dot[1] == '\0') return 0;
    return 1;
}

char* register_user(const char* username, const char* password, const char* first_name, const char* last_name, const char* email) {
    char* result_msg = NULL;
    char password_copy[128]; // Work with a mutable copy

    // Input Validation
    if (!username || !is_valid_username(username)) return strdup("Registration failed: Invalid username.");
    if (!password || !is_strong_password(password)) return strdup("Registration failed: Weak password.");
    if (strlen(password) >= sizeof(password_copy)) return strdup("Registration failed: Password too long.");
    if (!first_name || strlen(first_name) == 0 || strlen(first_name) > 50) return strdup("Registration failed: Invalid first name.");
    if (!last_name || strlen(last_name) == 0 || strlen(last_name) > 50) return strdup("Registration failed: Invalid last name.");
    if (!email || !is_valid_email(email)) return strdup("Registration failed: Invalid email.");
    
    strncpy(password_copy, password, sizeof(password_copy) - 1);
    password_copy[sizeof(password_copy) - 1] = '\0';

    User* current = user_database;
    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            secure_wipe(password_copy, sizeof(password_copy));
            return strdup("Registration failed: Username already exists.");
        }
        current = current->next;
    }

    unsigned char salt_bytes[SALT_SIZE];
    unsigned char hash_bytes[HASH_KEY_LENGTH];
    if (RAND_bytes(salt_bytes, sizeof(salt_bytes)) != 1) {
        secure_wipe(password_copy, sizeof(password_copy));
        return strdup("Registration failed: Could not generate salt.");
    }
    
    if (PKCS5_PBKDF2_HMAC(password_copy, strlen(password_copy), salt_bytes, sizeof(salt_bytes), HASH_ITERATIONS, EVP_sha256(), sizeof(hash_bytes), hash_bytes) != 1) {
        secure_wipe(password_copy, sizeof(password_copy));
        return strdup("Registration failed: Hashing failed.");
    }
    secure_wipe(password_copy, sizeof(password_copy));

    User* new_user = (User*)malloc(sizeof(User));
    if (!new_user) {
        return strdup("Registration failed: Memory allocation error.");
    }
    new_user->username = strdup(username);
    new_user->first_name = strdup(first_name);
    new_user->last_name = strdup(last_name);
    new_user->email = strdup(email);
    new_user->salt = bytes_to_hex(salt_bytes, sizeof(salt_bytes));
    new_user->hashed_password = bytes_to_hex(hash_bytes, sizeof(hash_bytes));
    new_user->next = NULL;

    if (!new_user->username || !new_user->first_name || !new_user->last_name || !new_user->email || !new_user->salt || !new_user->hashed_password) {
        free(new_user->username); free(new_user->first_name); free(new_user->last_name);
        free(new_user->email); free(new_user->salt); free(new_user->hashed_password);
        free(new_user);
        return strdup("Registration failed: Memory allocation error for user data.");
    }

    new_user->next = user_database;
    user_database = new_user;

    char msg_buffer[100];
    snprintf(msg_buffer, sizeof(msg_buffer), "Registration succeeded for username: %s", username);
    return strdup(msg_buffer);
}

void run_test(int test_num, const char* username, const char* password, const char* fname, const char* lname, const char* email) {
    char* result = register_user(username, password, fname, lname, email);
    printf("Test %d: %s\n", test_num, result);
    free(result);
}

int main() {
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Successful Registration
    run_test(1, "john_doe", "StrongPass@123", "John", "Doe", "john.doe@example.com");

    // Test Case 2: Username already exists
    run_test(2, "john_doe", "AnotherPass@456", "John", "Doe", "john.d@example.com");

    // Test Case 3: Weak password
    run_test(3, "jane_doe", "weak", "Jane", "Doe", "jane.doe@example.com");

    // Test Case 4: Invalid username
    run_test(4, "jd", "StrongPass@123", "Jane", "Doe", "jane.doe@example.com");

    // Test Case 5: Invalid email
    run_test(5, "peter_jones", "StrongPass@123", "Peter", "Jones", "peterjones@invalid");

    printf("\n--- Database Content (for verification) ---\n");
    User* current = user_database;
    while (current != NULL) {
        printf("Username: %s, Hashed Password: %.10s...\n", current->username, current->hashed_password);
        current = current->next;
    }

    free_user_list();
    return 0;
}