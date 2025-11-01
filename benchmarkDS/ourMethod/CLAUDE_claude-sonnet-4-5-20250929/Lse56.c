
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define MAX_EMAIL_LEN 254
#define MAX_PASSWORD_LEN 128
#define HASH_SIZE 32
#define SALT_SIZE 16
#define MAX_USERS 100

typedef struct {
    char email[MAX_EMAIL_LEN + 1];
    unsigned char password_hash[HASH_SIZE];
    unsigned char salt[SALT_SIZE];
    int logged_in;
} User;

User users[MAX_USERS];
int user_count = 0;

/* Generate cryptographically secure random salt */
int generate_salt(unsigned char* salt, size_t length) {
    if (RAND_bytes(salt, length) != 1) {
        return 0;
    }
    return 1;
}

/* Hash password using PBKDF2-HMAC-SHA256 with 210000 iterations */
void hash_password(const char* password, const unsigned char* salt, unsigned char* hash) {
    PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_SIZE, 
                      210000, EVP_sha256(), HASH_SIZE, hash);
}

/* Constant-time comparison to prevent timing attacks */
int constant_time_equals(const unsigned char* a, const unsigned char* b, size_t length) {
    unsigned char result = 0;
    size_t i;
    for (i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/* Validate email format */
int is_valid_email(const char* email) {
    size_t len;
    size_t i;
    int at_count = 0;
    int dot_after_at = 0;
    
    if (email == NULL) return 0;
    
    len = strlen(email);
    if (len == 0 || len > MAX_EMAIL_LEN) return 0;
    
    for (i = 0; i < len; i++) {
        if (email[i] == '@') {
            at_count++;
            if (i == 0 || i == len - 1) return 0;
        }
        if (at_count > 0 && email[i] == '.') {
            dot_after_at = 1;
        }
        if (!isalnum((unsigned char)email[i]) && 
            email[i] != '@' && email[i] != '.' && 
            email[i] != '_' && email[i] != '%' && 
            email[i] != '+' && email[i] != '-') {
            return 0;
        }
    }
    
    return at_count == 1 && dot_after_at;
}

/* Sanitize input to prevent injection attacks */
void sanitize_input(const char* input, char* output, size_t max_len) {
    size_t i, j = 0;
    if (input == NULL) {
        output[0] = '\\0';
        return;
    }
    
    for (i = 0; input[i] != '\\0' && j < max_len - 1; i++) {
        if (input[i] >= 0x20 && input[i] != 0x7F) {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

/* Find user by email */
User* find_user(const char* email) {
    int i;
    for (i = 0; i < user_count; i++) {
        if (strcmp(users[i].email, email) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

/* Register a new user */
const char* register_user(const char* email, const char* password) {
    char clean_email[MAX_EMAIL_LEN + 1];
    User* new_user;
    
    sanitize_input(email, clean_email, MAX_EMAIL_LEN + 1);
    
    if (!is_valid_email(clean_email)) {
        return "Error: Invalid email format";
    }
    
    if (password == NULL || strlen(password) < 8) {
        return "Error: Password must be at least 8 characters";
    }
    
    if (find_user(clean_email) != NULL) {
        return "Error: User already exists";
    }
    
    if (user_count >= MAX_USERS) {
        return "Error: Maximum users reached";
    }
    
    new_user = &users[user_count++];
    strncpy(new_user->email, clean_email, MAX_EMAIL_LEN);
    new_user->email[MAX_EMAIL_LEN] = '\\0';
    
    if (!generate_salt(new_user->salt, SALT_SIZE)) {
        user_count--;
        return "Error: Failed to generate salt";
    }
    
    hash_password(password, new_user->salt, new_user->password_hash);
    new_user->logged_in = 0;
    
    return "Success: User registered";
}

/* Login user */
const char* login(const char* email, const char* password) {
    char clean_email[MAX_EMAIL_LEN + 1];
    User* user;
    unsigned char provided_hash[HASH_SIZE];
    
    sanitize_input(email, clean_email, MAX_EMAIL_LEN + 1);
    
    if (!is_valid_email(clean_email)) {
        return "Error: Invalid credentials";
    }
    
    if (password == NULL || strlen(password) == 0) {
        return "Error: Invalid credentials";
    }
    
    user = find_user(clean_email);
    if (user == NULL) {
        return "Error: Invalid credentials";
    }
    
    hash_password(password, user->salt, provided_hash);
    
    if (!constant_time_equals(user->password_hash, provided_hash, HASH_SIZE)) {
        return "Error: Invalid credentials";
    }
    
    user->logged_in = 1;
    return "Success: Logged in";
}

/* Change email - requires authentication */
const char* change_email(const char* old_email, const char* new_email, const char* password) {
    char clean_old_email[MAX_EMAIL_LEN + 1];
    char clean_new_email[MAX_EMAIL_LEN + 1];
    User* user;
    unsigned char provided_hash[HASH_SIZE];
    int i;
    
    sanitize_input(old_email, clean_old_email, MAX_EMAIL_LEN + 1);
    sanitize_input(new_email, clean_new_email, MAX_EMAIL_LEN + 1);
    
    if (!is_valid_email(clean_old_email) || !is_valid_email(clean_new_email)) {
        return "Error: Invalid email format";
    }
    
    if (password == NULL || strlen(password) == 0) {
        return "Error: Password required";
    }
    
    user = find_user(clean_old_email);
    if (user == NULL) {
        return "Error: User not found";
    }
    
    if (!user->logged_in) {
        return "Error: User must be logged in";
    }
    
    hash_password(password, user->salt, provided_hash);
    
    if (!constant_time_equals(user->password_hash, provided_hash, HASH_SIZE)) {
        return "Error: Incorrect password";
    }
    
    if (strcmp(clean_old_email, clean_new_email) == 0) {
        return "Error: New email must be different from old email";
    }
    
    if (find_user(clean_new_email) != NULL) {
        return "Error: New email already in use";
    }
    
    strncpy(user->email, clean_new_email, MAX_EMAIL_LEN);
    user->email[MAX_EMAIL_LEN] = '\\0';
    
    return "Success: Email changed";
}

int main(void) {
    /* Test Case 1: Register and login */
    printf("Test 1: Register and Login\\n");
    printf("%s\\n", register_user("user1@example.com", "SecurePass123!"));
    printf("%s\\n", login("user1@example.com", "SecurePass123!"));
    
    /* Test Case 2: Change email with valid credentials */
    printf("\\nTest 2: Change email successfully\\n");
    printf("%s\\n", change_email("user1@example.com", "newuser1@example.com", "SecurePass123!"));
    
    /* Test Case 3: Try to change email without being logged in */
    printf("\\nTest 3: Change email without login\\n");
    printf("%s\\n", register_user("user2@example.com", "Password456!"));
    printf("%s\\n", change_email("user2@example.com", "newuser2@example.com", "Password456!"));
    
    /* Test Case 4: Try to change email with wrong password */
    printf("\\nTest 4: Change email with wrong password\\n");
    printf("%s\\n", register_user("user3@example.com", "MyPass789!"));
    printf("%s\\n", login("user3@example.com", "MyPass789!"));
    printf("%s\\n", change_email("user3@example.com", "newuser3@example.com", "WrongPassword"));
    
    /* Test Case 5: Try to change to same email */
    printf("\\nTest 5: Change to same email\\n");
    printf("%s\\n", register_user("user4@example.com", "TestPass000!"));
    printf("%s\\n", login("user4@example.com", "TestPass000!"));
    printf("%s\\n", change_email("user4@example.com", "user4@example.com", "TestPass000!"));
    
    return 0;
}
