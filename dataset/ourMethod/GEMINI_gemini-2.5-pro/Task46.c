// To compile: gcc -Wall -Wextra -pedantic your_file_name.c -o program -lssl -lcrypto
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// --- Constants ---
#define SALT_LEN 16
#define HASH_LEN 32
#define PBKDF2_ITERATIONS 210000
#define DB_FILE_C "users_c.db"
#define MAX_NAME_LEN 100
#define MAX_EMAIL_LEN 254
#define MAX_PASS_LEN 100

// --- Helper Functions ---

/**
 * @brief Converts binary data to a hexadecimal string.
 * @param data Pointer to the binary data.
 * @param data_len Length of the binary data.
 * @param hex_out Pointer to the output buffer for the hex string. Must be at least (2 * data_len + 1) bytes.
 * @param hex_out_len The size of the output buffer.
 * @return True on success, false on failure (e.g., buffer too small).
 */
bool bytes_to_hex(const unsigned char* data, size_t data_len, char* hex_out, size_t hex_out_len) {
    if (hex_out_len < (data_len * 2 + 1)) {
        return false;
    }
    for (size_t i = 0; i < data_len; i++) {
        // snprintf is safe and prevents buffer overflows.
        snprintf(hex_out + (i * 2), 3, "%02x", data[i]);
    }
    hex_out[data_len * 2] = '\0';
    return true;
}

/**
 * @brief Validates an email address format.
 * @param email The email address string to validate.
 * @return True if the email format is plausible, false otherwise.
 */
bool validate_email(const char* email) {
    if (email == NULL || strlen(email) > MAX_EMAIL_LEN) return false;
    const char *at_char = strchr(email, '@');
    if (at_char == NULL || at_char == email) return false; // Must have '@' not at the start
    const char *dot_char = strrchr(at_char, '.');
    if (dot_char == NULL || dot_char == at_char + 1 || dot_char[1] == '\0') return false; // Must have '.' after '@' with chars between and after
    return true;
}

/**
 * @brief Validates password complexity.
 * Policy: min 12 chars, 1 uppercase, 1 lowercase, 1 digit, 1 special char.
 * @param password The password string to validate.
 * @return True if the password meets complexity requirements, false otherwise.
 */
bool validate_password(const char* password) {
    if (password == NULL || strlen(password) < 12 || strlen(password) > MAX_PASS_LEN) return false;

    bool has_upper = false, has_lower = false, has_digit = false, has_special = false;
    const char* special_chars = "@$!%*?&";
    
    for (size_t i = 0; i < strlen(password); i++) {
        if (isupper((unsigned char)password[i])) has_upper = true;
        else if (islower((unsigned char)password[i])) has_lower = true;
        else if (isdigit((unsigned char)password[i])) has_digit = true;
        else if (strchr(special_chars, password[i])) has_special = true;
    }
    
    return has_upper && has_lower && has_digit && has_special;
}

/**
 * @brief Hashes a password using PBKDF2-HMAC-SHA256.
 * @param password The plaintext password.
 * @param salt Buffer to store the generated salt. Must be at least SALT_LEN bytes.
 * @param hash Buffer to store the resulting hash. Must be at least HASH_LEN bytes.
 * @return True on success, false on failure.
 */
bool hash_password(const char* password, unsigned char* salt, unsigned char* hash) {
    if (RAND_bytes(salt, SALT_LEN) != 1) {
        fprintf(stderr, "Error: Failed to generate salt.\n");
        return false;
    }

    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_LEN,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        HASH_LEN,
        hash
    );
    
    if (result != 1) {
        fprintf(stderr, "Error: PKCS5_PBKDF2_HMAC failed.\n");
        return false;
    }

    return true;
}

/**
 * @brief Registers a new user by storing their details in a file.
 * @param name The user's name.
 * @param email The user's email.
 * @param password The user's plaintext password.
 * @return True if registration is successful, false otherwise.
 */
bool register_user(const char* name, const char* email, const char* password) {
    // 1. Validate inputs
    if (name == NULL || name[0] == '\0' || strlen(name) > MAX_NAME_LEN) {
        fprintf(stderr, "Registration failed: Invalid name.\n");
        return false;
    }
    if (!validate_email(email)) {
        fprintf(stderr, "Registration failed: Invalid email format.\n");
        return false;
    }
    if (!validate_password(password)) {
        fprintf(stderr, "Registration failed: Password does not meet complexity requirements.\n");
        fprintf(stderr, "Policy: min 12 chars, 1 uppercase, 1 lowercase, 1 digit, 1 special char (@$!%%*?&).\n");
        return false;
    }

    // 2. Hash password
    unsigned char salt[SALT_LEN];
    unsigned char hash[HASH_LEN];
    if (!hash_password(password, salt, hash)) {
        return false;
    }

    char salt_hex[SALT_LEN * 2 + 1];
    char hash_hex[HASH_LEN * 2 + 1];
    if (!bytes_to_hex(salt, SALT_LEN, salt_hex, sizeof(salt_hex)) ||
        !bytes_to_hex(hash, HASH_LEN, hash_hex, sizeof(hash_hex))) {
        fprintf(stderr, "Error: Failed to convert binary to hex.\n");
        return false;
    }

    // 3. Store in "database" (append to a file)
    FILE *db_file = fopen(DB_FILE_C, "a");
    if (db_file == NULL) {
        perror("Error opening database file");
        return false;
    }
    
    fprintf(db_file, "%s,%s,%s,%s\n", name, email, salt_hex, hash_hex);
    fclose(db_file);

    // Securely clear sensitive data from memory
    memset(salt, 0, SALT_LEN);
    memset(hash, 0, HASH_LEN);
    
    return true;
}

int main() {
    printf("User Registration System (C version)\n");
    printf("User data will be stored in '%s'\n", DB_FILE_C);
    printf("----------------------------------\n");

    // Test Case 1: Successful registration
    printf("Test Case 1: Registering a valid user 'Alice'.\n");
    if (register_user("Alice", "alice@example.com", "Password123!@#")) {
        printf("SUCCESS: Alice registered.\n");
    } else {
        printf("FAILURE: Could not register Alice.\n");
    }
    printf("----------------------------------\n");
    
    // Test Case 2: Successful registration
    printf("Test Case 2: Registering a valid user 'Bob'.\n");
    if (register_user("Bob", "bob@example.com", "SecurePass$5678")) {
        printf("SUCCESS: Bob registered.\n");
    } else {
        printf("FAILURE: Could not register Bob.\n");
    }
    printf("----------------------------------\n");

    // Test Case 3: Invalid email
    printf("Test Case 3: Attempting to register with an invalid email.\n");
    if (register_user("Charlie", "charlie@invalid", "Password123!@#")) {
        printf("FAILURE: Charlie was registered (this should not happen).\n");
    } else {
        printf("SUCCESS: Registration failed as expected for invalid email.\n");
    }
    printf("----------------------------------\n");

    // Test Case 4: Weak password
    printf("Test Case 4: Attempting to register with a weak password.\n");
    if (register_user("David", "david@example.com", "weak")) {
        printf("FAILURE: David was registered (this should not happen).\n");
    } else {
        printf("SUCCESS: Registration failed as expected for weak password.\n");
    }
    printf("----------------------------------\n");

    // Test Case 5: Empty name
    printf("Test Case 5: Attempting to register with an empty name.\n");
    if (register_user("", "eve@example.com", "AnotherGoodPass1!")) {
        printf("FAILURE: User with empty name was registered (this should not happen).\n");
    } else {
        printf("SUCCESS: Registration failed as expected for empty name.\n");
    }
    printf("----------------------------------\n");

    return 0;
}