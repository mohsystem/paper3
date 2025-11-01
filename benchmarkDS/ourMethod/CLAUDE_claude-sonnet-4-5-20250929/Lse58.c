
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

/* Security constants */
#define MIN_PASSWORD_LENGTH 8
#define SALT_LENGTH 16
#define HASH_LENGTH 32
#define PBKDF2_ITERATIONS 210000
#define SESSION_TOKEN_LENGTH 32
#define MAX_EMAIL_LENGTH 254
#define MAX_PASSWORD_LENGTH 128
#define MAX_USERS 100

/* User account structure */
typedef struct {
    char email[MAX_EMAIL_LENGTH + 1];
    unsigned char password_hash[HASH_LENGTH];
    unsigned char salt[SALT_LENGTH];
    int active;
} UserAccount;

/* Session structure */
typedef struct {
    char token[SESSION_TOKEN_LENGTH * 2 + 1];
    char email[MAX_EMAIL_LENGTH + 1];
    int active;
} Session;

/* In-memory stores */
static UserAccount user_store[MAX_USERS];
static Session session_store[MAX_USERS];
static int user_count = 0;
static int session_count = 0;

/* Securely clear memory */
void secure_zero(void* ptr, size_t size) {
    if (ptr) {
        volatile unsigned char* p = (volatile unsigned char*)ptr;
        while (size--) {
            *p++ = 0;
        }
    }
}

/* Generate cryptographically secure random bytes */
int generate_random_bytes(unsigned char* buffer, int length) {
    if (!buffer || length <= 0) {
        return 0;
    }
    return RAND_bytes(buffer, length) == 1;
}

/* Hash password using PBKDF2-HMAC-SHA-256 */
int hash_password(const char* password, const unsigned char* salt, 
                  unsigned char* output) {
    if (!password || !salt || !output) {
        return 0;
    }
    
    int result = PKCS5_PBKDF2_HMAC(
        password, strlen(password),
        salt, SALT_LENGTH,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        HASH_LENGTH, output
    );
    
    return result == 1;
}

/* Constant-time comparison */
int constant_time_compare(const unsigned char* a, const unsigned char* b, size_t length) {
    if (!a || !b) {
        return 0;
    }
    
    unsigned char result = 0;
    for (size_t i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

/* Validate email format using regex */
int is_valid_email(const char* email) {
    if (!email || strlen(email) == 0 || strlen(email) > MAX_EMAIL_LENGTH) {
        return 0;
    }
    
    regex_t regex;
    int ret = regcomp(&regex, "^[a-zA-Z0-9_+&*-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,7}$", 
                     REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        return 0;
    }
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return ret == 0;
}

/* Validate password strength */
int is_valid_password(const char* password) {
    if (!password || strlen(password) < MIN_PASSWORD_LENGTH || 
        strlen(password) > MAX_PASSWORD_LENGTH) {
        return 0;
    }
    
    int has_digit = 0;
    int has_lower = 0;
    int has_upper = 0;
    
    for (const char* p = password; *p; p++) {
        if (isdigit(*p)) has_digit = 1;
        if (islower(*p)) has_lower = 1;
        if (isupper(*p)) has_upper = 1;
    }
    
    return has_digit && has_lower && has_upper;
}

/* Find user by email */
UserAccount* find_user(const char* email) {
    if (!email) {
        return NULL;
    }
    
    for (int i = 0; i < user_count; i++) {
        if (user_store[i].active && strcmp(user_store[i].email, email) == 0) {
            return &user_store[i];
        }
    }
    return NULL;
}

/* Generate secure session token */
int generate_session_token(char* output) {
    if (!output) {
        return 0;
    }
    
    unsigned char token_bytes[SESSION_TOKEN_LENGTH];
    if (!generate_random_bytes(token_bytes, SESSION_TOKEN_LENGTH)) {
        return 0;
    }
    
    for (int i = 0; i < SESSION_TOKEN_LENGTH; i++) {
        sprintf(output + i * 2, "%02x", token_bytes[i]);
    }
    output[SESSION_TOKEN_LENGTH * 2] = '\\0';
    
    return 1;
}

/* Register a new user */
const char* register_user(const char* email, const char* password) {
    /* Validate input */
    if (!email || !password) {
        return "Error: Invalid input";
    }
    
    if (!is_valid_email(email)) {
        return "Error: Invalid email format";
    }
    
    if (!is_valid_password(password)) {
        return "Error: Password must be at least 8 characters and contain uppercase, lowercase, and digit";
    }
    
    /* Check if user already exists */
    if (find_user(email) != NULL) {
        return "Error: User already exists";
    }
    
    /* Check capacity */
    if (user_count >= MAX_USERS) {
        return "Error: User limit reached";
    }
    
    /* Create new user */
    UserAccount* account = &user_store[user_count];
    strncpy(account->email, email, MAX_EMAIL_LENGTH);
    account->email[MAX_EMAIL_LENGTH] = '\\0';
    
    /* Generate unique salt (mitigates CWE-759) */
    if (!generate_random_bytes(account->salt, SALT_LENGTH)) {
        return "Error: Failed to generate salt";
    }
    
    /* Hash password */
    if (!hash_password(password, account->salt, account->password_hash)) {
        return "Error: Failed to hash password";
    }
    
    account->active = 1;
    user_count++;
    
    return "Success: User registered";
}

/* Login and create session */
const char* login(const char* email, const char* password, char* token_out) {
    /* Validate input */
    if (!email || !password || !token_out) {
        return NULL;
    }
    
    /* Find user */
    UserAccount* account = find_user(email);
    if (!account) {
        return NULL;  /* Generic error */
    }
    
    /* Verify password */
    unsigned char provided_hash[HASH_LENGTH];
    if (!hash_password(password, account->salt, provided_hash)) {
        return NULL;
    }
    
    if (!constant_time_compare(provided_hash, account->password_hash, HASH_LENGTH)) {
        secure_zero(provided_hash, sizeof(provided_hash));
        return NULL;
    }
    
    secure_zero(provided_hash, sizeof(provided_hash));
    
    /* Check session capacity */
    if (session_count >= MAX_USERS) {
        return NULL;
    }
    
    /* Create session */
    Session* session = &session_store[session_count];
    if (!generate_session_token(session->token)) {
        return NULL;
    }
    
    strncpy(session->email, email, MAX_EMAIL_LENGTH);
    session->email[MAX_EMAIL_LENGTH] = '\\0';
    session->active = 1;
    session_count++;
    
    strncpy(token_out, session->token, SESSION_TOKEN_LENGTH * 2 + 1);
    
    return "Success";
}

/* Get email from session token */
const char* get_email_from_session(const char* token) {
    if (!token) {
        return NULL;
    }
    
    for (int i = 0; i < session_count; i++) {
        if (session_store[i].active && strcmp(session_store[i].token, token) == 0) {
            return session_store[i].email;
        }
    }
    return NULL;
}

/* Change email */
const char* change_email(const char* session_token, const char* old_email,
                        const char* new_email, const char* password) {
    /* Validate input */
    if (!session_token || !old_email || !new_email || !password) {
        return "Error: Invalid input";
    }
    
    /* Verify session */
    const char* session_email = get_email_from_session(session_token);
    if (!session_email) {
        return "Error: Not authenticated";
    }
    
    /* Verify old email matches session */
    if (strcmp(session_email, old_email) != 0) {
        return "Error: Email mismatch";
    }
    
    /* Validate new email */
    if (!is_valid_email(new_email)) {
        return "Error: Invalid new email format";
    }
    
    /* Check if new email is taken */
    UserAccount* existing = find_user(new_email);
    if (existing && strcmp(new_email, old_email) != 0) {
        return "Error: Email already in use";
    }
    
    /* Find user account */
    UserAccount* account = find_user(old_email);
    if (!account) {
        return "Error: User not found";
    }
    
    /* Verify password */
    unsigned char provided_hash[HASH_LENGTH];
    if (!hash_password(password, account->salt, provided_hash)) {
        return "Error: Failed to verify password";
    }
    
    if (!constant_time_compare(provided_hash, account->password_hash, HASH_LENGTH)) {
        secure_zero(provided_hash, sizeof(provided_hash));
        return "Error: Invalid password";
    }
    
    secure_zero(provided_hash, sizeof(provided_hash));
    
    /* Update email */
    strncpy(account->email, new_email, MAX_EMAIL_LENGTH);
    account->email[MAX_EMAIL_LENGTH] = '\\0';
    
    /* Update session */
    for (int i = 0; i < session_count; i++) {
        if (session_store[i].active && strcmp(session_store[i].token, session_token) == 0) {
            strncpy(session_store[i].email, new_email, MAX_EMAIL_LENGTH);
            session_store[i].email[MAX_EMAIL_LENGTH] = '\\0';
            break;
        }
    }
    
    return "Success: Email changed";
}

int main(void) {
    char token[SESSION_TOKEN_LENGTH * 2 + 1];
    
    printf("=== Test Case 1: Register and change email successfully ===\\n");
    printf("Register: %s\\n", register_user("user1@example.com", "SecurePass123"));
    const char* login_result1 = login("user1@example.com", "SecurePass123", token);
    printf("Login: %s\\n", login_result1 ? "Success" : "Failed");
    printf("Change email: %s\\n\\n", 
           change_email(token, "user1@example.com", "newemail1@example.com", "SecurePass123"));
    
    printf("=== Test Case 2: Wrong old email ===\\n");
    printf("Register: %s\\n", register_user("user2@example.com", "StrongPass456"));
    login("user2@example.com", "StrongPass456", token);
    printf("Change email with wrong old email: %s\\n\\n",
           change_email(token, "wrong@example.com", "new2@example.com", "StrongPass456"));
    
    printf("=== Test Case 3: Wrong password ===\\n");
    printf("Register: %s\\n", register_user("user3@example.com", "MyPassword789"));
    login("user3@example.com", "MyPassword789", token);
    printf("Change email with wrong password: %s\\n\\n",
           change_email(token, "user3@example.com", "new3@example.com", "WrongPass123"));
    
    printf("=== Test Case 4: Not logged in ===\\n");
    printf("Change email without login: %s\\n\\n",
           change_email("invalid_token", "user4@example.com", "new4@example.com", "AnyPass123"));
    
    printf("=== Test Case 5: Invalid new email format ===\\n");
    printf("Register: %s\\n", register_user("user5@example.com", "ValidPass999"));
    login("user5@example.com", "ValidPass999", token);
    printf("Change to invalid email: %s\\n",
           change_email(token, "user5@example.com", "invalid-email", "ValidPass999"));
    
    return 0;
}
