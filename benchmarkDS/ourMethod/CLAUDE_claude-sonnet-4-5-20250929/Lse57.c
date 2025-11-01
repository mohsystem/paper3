
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define MAX_EMAIL_LEN 255
#define MAX_PASSWORD_LEN 128
#define SALT_LEN 16
#define HASH_LEN 32
#define MAX_USERS 100

/* User record structure */
typedef struct {
    char email[MAX_EMAIL_LEN];
    char password_hash[HASH_LEN * 2 + 1];
    unsigned char salt[SALT_LEN];
    int is_active;
} UserRecord;

/* Simulated user database */
typedef struct {
    char user_id[64];
    UserRecord record;
} UserEntry;

static UserEntry user_database[MAX_USERS];
static int user_count = 0;

/* Generate cryptographically secure salt - Rule#8, Rule#9 */
int generate_salt(unsigned char* salt, size_t len) {
    if (!salt) return 0;
    if (RAND_bytes(salt, len) != 1) {
        return 0;
    }
    return 1;
}

/* Hash password with PBKDF2-HMAC-SHA256 - Rule#6, Rule#15 */
int hash_password(const char* password, const unsigned char* salt, char* out_hash, size_t out_len) {
    if (!password || !salt || !out_hash || out_len < (HASH_LEN * 2 + 1)) {
        return 0;
    }
    
    unsigned char hash[HASH_LEN];
    /* Use PBKDF2 with at least 210000 iterations */
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, SALT_LEN, 210000,
                          EVP_sha256(), HASH_LEN, hash) != 1) {
        return 0;
    }
    
    /* Convert to hex string */
    for (int i = 0; i < HASH_LEN; i++) {
        snprintf(out_hash + (i * 2), 3, "%02x", hash[i]);
    }
    out_hash[HASH_LEN * 2] = '\\0';
    
    /* Clear sensitive data - Rule#1 */
    OPENSSL_cleanse(hash, sizeof(hash));
    
    return 1;
}

/* Constant-time comparison to prevent timing attacks - Rule#6 */
int constant_time_equals(const char* a, const char* b) {
    if (!a || !b) return 0;
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    if (len_a != len_b) return 0;
    return CRYPTO_memcmp(a, b, len_a) == 0;
}

/* Validate email format - Rule#1, Rule#13 */
int is_valid_email(const char* email) {
    if (!email || strlen(email) == 0 || strlen(email) > 254) return 0;
    
    int has_at = 0, has_dot_after_at = 0;
    const char* at_pos = NULL;
    
    for (const char* p = email; *p; p++) {
        if (*p == '@') {
            if (has_at) return 0; /* Multiple @ symbols */
            has_at = 1;
            at_pos = p;
        } else if (has_at && *p == '.' && p > at_pos + 1) {
            has_dot_after_at = 1;
        }
    }
    
    return has_at && has_dot_after_at && at_pos != email && *(at_pos + 1) != '\\0';
}

/* Find user by ID */
UserRecord* find_user(const char* user_id) {
    if (!user_id) return NULL;
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i].user_id, user_id) == 0 && 
            user_database[i].record.is_active) {
            return &user_database[i].record;
        }
    }
    return NULL;
}

/* Check if email exists */
int email_exists(const char* email, const char* exclude_user_id) {
    if (!email) return 0;
    for (int i = 0; i < user_count; i++) {
        if (user_database[i].record.is_active &&
            strcmp(user_database[i].record.email, email) == 0 &&
            (!exclude_user_id || strcmp(user_database[i].user_id, exclude_user_id) != 0)) {
            return 1;
        }
    }
    return 0;
}

/* Change email securely - Rule#1, Rule#2, Rule#16 */
const char* change_email(const char* session_user_id, 
                        const char* old_email,
                        const char* new_email, 
                        const char* confirm_password) {
    /* Input validation - Rule#13 */
    if (!session_user_id || strlen(session_user_id) == 0) {
        return "Error: Not authenticated";
    }
    
    if (!old_email || !new_email || !confirm_password ||
        strlen(old_email) == 0 || strlen(new_email) == 0 || strlen(confirm_password) == 0) {
        return "Error: All fields required";
    }
    
    /* Validate email formats - Rule#13 */
    if (!is_valid_email(old_email) || !is_valid_email(new_email)) {
        return "Error: Invalid email format";
    }
    
    /* Check email length limits - Rule#14 */
    if (strlen(old_email) > 254 || strlen(new_email) > 254) {
        return "Error: Email too long";
    }
    
    /* Prevent same old and new email */
    if (strcmp(old_email, new_email) == 0) {
        return "Error: New email must be different";
    }
    
    /* Retrieve user record */
    UserRecord* user = find_user(session_user_id);
    if (!user) {
        return "Error: User not found";
    }
    
    /* Verify old email matches current email - Rule#1 */
    if (!constant_time_equals(user->email, old_email)) {
        return "Error: Old email incorrect";
    }
    
    /* Verify password - Rule#6, Rule#15 */
    char provided_hash[HASH_LEN * 2 + 1];
    if (!hash_password(confirm_password, user->salt, provided_hash, sizeof(provided_hash))) {
        return "Error: Password verification failed";
    }
    
    if (!constant_time_equals(user->password_hash, provided_hash)) {
        /* Clear sensitive data */
        OPENSSL_cleanse(provided_hash, sizeof(provided_hash));
        return "Error: Password incorrect";
    }
    
    /* Clear password hash from memory */
    OPENSSL_cleanse(provided_hash, sizeof(provided_hash));
    
    /* Check if new email already exists */
    if (email_exists(new_email, session_user_id)) {
        return "Error: Email already in use";
    }
    
    /* Update email - Rule#14 bounds check */
    strncpy(user->email, new_email, MAX_EMAIL_LEN - 1);
    user->email[MAX_EMAIL_LEN - 1] = '\\0';
    
    return "Success: Email changed";
}

/* Helper to create test user */
int create_user(const char* user_id, const char* email, const char* password) {
    if (user_count >= MAX_USERS) return 0;
    if (!user_id || !email || !password) return 0;
    
    unsigned char salt[SALT_LEN];
    if (!generate_salt(salt, SALT_LEN)) return 0;
    
    char hash[HASH_LEN * 2 + 1];
    if (!hash_password(password, salt, hash, sizeof(hash))) return 0;
    
    strncpy(user_database[user_count].user_id, user_id, 63);
    user_database[user_count].user_id[63] = '\\0';
    
    strncpy(user_database[user_count].record.email, email, MAX_EMAIL_LEN - 1);
    user_database[user_count].record.email[MAX_EMAIL_LEN - 1] = '\\0';
    
    strncpy(user_database[user_count].record.password_hash, hash, sizeof(hash) - 1);
    user_database[user_count].record.password_hash[sizeof(hash) - 1] = '\\0';
    
    memcpy(user_database[user_count].record.salt, salt, SALT_LEN);
    user_database[user_count].record.is_active = 1;
    
    user_count++;
    return 1;
}

int main(void) {
    printf("=== Email Change System Test Cases ===\\n\\n");
    
    /* Setup test users */
    create_user("user1", "alice@example.com", "SecurePass123!");
    create_user("user2", "bob@example.com", "AnotherPass456#");
    
    /* Test 1: Successful email change */
    printf("Test 1: Valid email change\\n");
    const char* result1 = change_email("user1", "alice@example.com", "alice.new@example.com", "SecurePass123!");
    printf("%s\\n\\n", result1);
    
    /* Test 2: Wrong old email */
    printf("Test 2: Wrong old email\\n");
    const char* result2 = change_email("user2", "wrong@example.com", "bob.new@example.com", "AnotherPass456#");
    printf("%s\\n\\n", result2);
    
    /* Test 3: Wrong password */
    printf("Test 3: Wrong password\\n");
    const char* result3 = change_email("user2", "bob@example.com", "bob.new@example.com", "WrongPassword!");
    printf("%s\\n\\n", result3);
    
    /* Test 4: Invalid email format */
    printf("Test 4: Invalid email format\\n");
    const char* result4 = change_email("user1", "alice.new@example.com", "invalid-email", "SecurePass123!");
    printf("%s\\n\\n", result4);
    
    /* Test 5: Empty fields */
    printf("Test 5: Empty fields\\n");
    const char* result5 = change_email("user2", "", "bob.final@example.com", "AnotherPass456#");
    printf("%s\\n\\n", result5);
    
    return 0;
}
