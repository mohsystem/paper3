#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>

#define SALT_BYTES 16
#define PBKDF2_ITERATIONS 310000
#define HASH_BYTES 32
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 100
#define MAX_STORED_CRED_LEN (SALT_BYTES * 2 + HASH_BYTES * 2 + 2) // salt_hex + ':' + hash_hex + '\0'
#define MAX_USERS 10

typedef struct {
    char username[MAX_USERNAME_LEN];
    char credentials[MAX_STORED_CRED_LEN];
} User;

// In-memory user database simulation.
static User user_database[MAX_USERS];
static int user_count = 0;

// Helper to convert byte array to hex string
void to_hex(const unsigned char* data, size_t len, char* hex_string) {
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_string + (i * 2), "%02x", data[i]);
    }
    hex_string[len * 2] = '\0';
}

// Helper to convert hex string to byte array
void from_hex(const char* hex_string, unsigned char* data, size_t data_len) {
    for (size_t i = 0; i < data_len; ++i) {
        sscanf(hex_string + (i * 2), "%2hhx", &data[i]);
    }
}

// Securely clears memory
void secure_zero_memory(void* v, size_t n) {
    volatile unsigned char* p = (volatile unsigned char*)v;
    while (n--) {
        *p++ = 0;
    }
}

int hash_password(const char* password, const unsigned char* salt, unsigned char* hash) {
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_BYTES,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        HASH_BYTES,
        hash
    );
    return result == 1;
}

void register_user(const char* username, const char* password) {
    if (user_count >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return;
    }
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        printf("Registration failed: Username and password cannot be empty.\n");
        return;
    }
    if (strlen(username) >= MAX_USERNAME_LEN) {
        printf("Registration failed: Username is too long.\n");
        return;
    }
    
    unsigned char salt[SALT_BYTES];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error: Failed to generate random salt.\n");
        return;
    }
    
    unsigned char hashed_pw[HASH_BYTES];
    if (!hash_password(password, salt, hashed_pw)) {
        fprintf(stderr, "Error: Failed to hash password.\n");
        return;
    }
    
    char salt_hex[SALT_BYTES * 2 + 1];
    char hash_hex[HASH_BYTES * 2 + 1];
    to_hex(salt, SALT_BYTES, salt_hex);
    to_hex(hashed_pw, HASH_BYTES, hash_hex);
    
    strncpy(user_database[user_count].username, username, MAX_USERNAME_LEN - 1);
    user_database[user_count].username[MAX_USERNAME_LEN - 1] = '\0';

    snprintf(user_database[user_count].credentials, MAX_STORED_CRED_LEN, "%s:%s", salt_hex, hash_hex);
    
    user_count++;
    printf("User '%s' registered successfully.\n", username);

    secure_zero_memory(hashed_pw, HASH_BYTES);
}

const char* login_user(const char* username, const char* password) {
    // Rule #1: Input validation
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return "Redirecting to login page: Invalid input.";
    }

    User* user = NULL;
    for (int i = 0; i < user_count; ++i) {
        if (strncmp(user_database[i].username, username, MAX_USERNAME_LEN) == 0) {
            user = &user_database[i];
            break;
        }
    }

    if (user == NULL) {
        return "Redirecting to login page: Invalid credentials.";
    }
    
    const char* delimiter = strchr(user->credentials, ':');
    if (delimiter == NULL) {
        return "Redirecting to login page: Server error (invalid credential format).";
    }

    size_t salt_hex_len = delimiter - user->credentials;
    if (salt_hex_len != SALT_BYTES * 2) {
        return "Redirecting to login page: Server error (malformed salt).";
    }
    
    char salt_hex[SALT_BYTES * 2 + 1];
    strncpy(salt_hex, user->credentials, salt_hex_len);
    salt_hex[salt_hex_len] = '\0';
    
    const char* stored_hash_hex = delimiter + 1;

    unsigned char salt[SALT_BYTES];
    from_hex(salt_hex, salt, SALT_BYTES);
    
    unsigned char stored_hash[HASH_BYTES];
    from_hex(stored_hash_hex, stored_hash, HASH_BYTES);

    unsigned char provided_hash[HASH_BYTES];
    if (!hash_password(password, salt, provided_hash)) {
        secure_zero_memory(provided_hash, HASH_BYTES);
        secure_zero_memory(stored_hash, HASH_BYTES);
        return "Redirecting to login page: Server error (hashing failed).";
    }

    // Constant-time comparison to prevent timing attacks
    int diff = CRYPTO_memcmp(stored_hash, provided_hash, HASH_BYTES);
    
    secure_zero_memory(provided_hash, HASH_BYTES);
    secure_zero_memory(stored_hash, HASH_BYTES);

    if (diff == 0) {
        return "Login successful! Redirecting to dashboard page.";
    } else {
        return "Redirecting to login page: Invalid credentials.";
    }
}


int main() {
    register_user("admin", "S3cureP@ssword!23");
    register_user("testuser", "password123");

    printf("\n--- Running Test Cases ---\n");
    
    // Test Case 1: Correct credentials
    printf("1. Testing with correct credentials (admin):\n");
    printf("   Result: %s\n", login_user("admin", "S3cureP@ssword!23"));
    
    // Test Case 2: Correct username, incorrect password
    printf("\n2. Testing with incorrect password (admin):\n");
    printf("   Result: %s\n", login_user("admin", "wrongpassword"));
    
    // Test Case 3: Incorrect username
    printf("\n3. Testing with incorrect username:\n");
    printf("   Result: %s\n", login_user("unknownuser", "S3cureP@ssword!23"));

    // Test Case 4: Correct username, empty password
    printf("\n4. Testing with empty password:\n");
    printf("   Result: %s\n", login_user("admin", ""));

    // Test Case 5: Empty username
    printf("\n5. Testing with empty username:\n");
    printf("   Result: %s\n", login_user("", "S3cureP@ssword!23"));

    return 0;
}