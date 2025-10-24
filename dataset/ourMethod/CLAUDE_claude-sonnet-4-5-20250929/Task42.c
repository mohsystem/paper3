
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

#define SALT_SIZE 16
#define HASH_SIZE 32
#define PBKDF2_ITERATIONS 210000
#define MAX_USERNAME_LEN 32
#define MIN_USERNAME_LEN 3
#define MAX_PASSWORD_LEN 128
#define MIN_PASSWORD_LEN 8
#define MAX_USERS 100

/* User credential structure with salt and hash */
typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];
    int active;
} UserCredential;

/* Simple in-memory user database */
typedef struct {
    UserCredential users[MAX_USERS];
    int userCount;
} UserDatabase;

/* Initialize database */
void initDatabase(UserDatabase* db) {
    if (db == NULL) {
        return;
    }
    memset(db, 0, sizeof(UserDatabase));
    db->userCount = 0;
}

/* Generate cryptographically secure random salt */
int generateSalt(unsigned char* salt) {
    if (salt == NULL) {
        return 0;
    }
    
    /* Use OpenSSL CSPRNG for secure randomness */
    if (RAND_bytes(salt, SALT_SIZE) != 1) {
        return 0;
    }
    return 1;
}

/* Derive key from password using PBKDF2-HMAC-SHA256 */
int deriveKey(const char* password, size_t passwordLen,
              const unsigned char* salt, unsigned char* hash) {
    if (password == NULL || salt == NULL || hash == NULL || passwordLen == 0) {
        return 0;
    }
    
    /* Validate password length to prevent integer overflow */
    if (passwordLen > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    /* Use PBKDF2 with SHA256 for key derivation */
    if (PKCS5_PBKDF2_HMAC(password, (int)passwordLen,
                          salt, SALT_SIZE,
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          HASH_SIZE,
                          hash) != 1) {
        return 0;
    }
    
    return 1;
}

/* Constant-time comparison to prevent timing attacks */
int constantTimeCompare(const unsigned char* a, const unsigned char* b, size_t len) {
    if (a == NULL || b == NULL || len == 0) {
        return 0;
    }
    
    /* Use OpenSSL constant-time comparison */
    return CRYPTO_memcmp(a, b, len) == 0;
}

/* Validate username format */
int validateUsername(const char* username) {
    size_t len;
    size_t i;
    
    if (username == NULL) {
        return 0;
    }
    
    len = strlen(username);
    
    /* Username must be 3-32 characters */
    if (len < MIN_USERNAME_LEN || len > MAX_USERNAME_LEN) {
        return 0;
    }
    
    /* Only alphanumeric and underscore allowed */
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

/* Validate password strength */
int validatePassword(const char* password) {
    size_t len;
    size_t i;
    int hasUpper = 0, hasLower = 0, hasDigit = 0, hasSpecial = 0;
    int complexity;
    
    if (password == NULL) {
        return 0;
    }
    
    len = strlen(password);
    
    /* Password must be 8-128 characters */
    if (len < MIN_PASSWORD_LEN || len > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    /* Check for character complexity */
    for (i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) {
            hasUpper = 1;
        } else if (islower(c)) {
            hasLower = 1;
        } else if (isdigit(c)) {
            hasDigit = 1;
        } else if (ispunct(c)) {
            hasSpecial = 1;
        }
    }
    
    /* Require at least 3 of 4 character types */
    complexity = hasUpper + hasLower + hasDigit + hasSpecial;
    return complexity >= 3;
}

/* Find user in database */
UserCredential* findUser(UserDatabase* db, const char* username) {
    int i;
    
    if (db == NULL || username == NULL) {
        return NULL;
    }
    
    for (i = 0; i < db->userCount; i++) {
        if (db->users[i].active && 
            strcmp(db->users[i].username, username) == 0) {
            return &db->users[i];
        }
    }
    
    return NULL;
}

/* Register new user with secure password storage */
int registerUser(UserDatabase* db, const char* username, const char* password) {
    UserCredential* user = NULL;
    size_t passwordLen;
    
    /* Validate inputs (treat all inputs as untrusted) */
    if (db == NULL || username == NULL || password == NULL) {
        fprintf(stderr, "Invalid input parameters\\n");
        return 0;
    }
    
    if (!validateUsername(username)) {
        fprintf(stderr, "Invalid username format\\n");
        return 0;
    }
    
    passwordLen = strlen(password);
    if (!validatePassword(password)) {
        fprintf(stderr, "Password does not meet complexity requirements\\n");
        return 0;
    }
    
    /* Check if user already exists */
    if (findUser(db, username) != NULL) {
        fprintf(stderr, "User already exists\\n");
        return 0;
    }
    
    /* Check database capacity */
    if (db->userCount >= MAX_USERS) {
        fprintf(stderr, "Database full\\n");
        return 0;
    }
    
    user = &db->users[db->userCount];
    
    /* Copy username with bounds check */
    strncpy(user->username, username, MAX_USERNAME_LEN);
    user->username[MAX_USERNAME_LEN] = '\\0';
    
    /* Generate unique salt for this user */
    if (!generateSalt(user->salt)) {
        fprintf(stderr, "Failed to generate salt\\n");
        return 0;
    }
    
    /* Derive password hash using PBKDF2 */
    if (!deriveKey(password, passwordLen, user->salt, user->hash)) {
        fprintf(stderr, "Failed to derive key\\n");
        /* Securely clear partial data */
        memset(user, 0, sizeof(UserCredential));
        return 0;
    }
    
    user->active = 1;
    db->userCount++;
    
    /* Never log or print passwords or hashes */
    return 1;
}

/* Authenticate user with constant-time comparison */
int authenticate(UserDatabase* db, const char* username, const char* password) {
    UserCredential* user = NULL;
    unsigned char derivedHash[HASH_SIZE];
    size_t passwordLen;
    int result;
    unsigned char dummySalt[SALT_SIZE];
    unsigned char dummyHash[HASH_SIZE];
    
    /* Validate inputs */
    if (db == NULL || username == NULL || password == NULL) {
        return 0;
    }
    
    if (!validateUsername(username)) {
        return 0;
    }
    
    passwordLen = strlen(password);
    if (passwordLen == 0 || passwordLen > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    /* Find user */
    user = findUser(db, username);
    if (user == NULL) {
        /* Perform dummy operation to prevent timing attacks */
        if (RAND_bytes(dummySalt, SALT_SIZE) == 1) {
            deriveKey(password, passwordLen, dummySalt, dummyHash);
        }
        return 0;
    }
    
    /* Initialize derived hash buffer */
    memset(derivedHash, 0, HASH_SIZE);
    
    /* Derive hash from provided password */
    if (!deriveKey(password, passwordLen, user->salt, derivedHash)) {
        /* Securely clear sensitive data */
        memset(derivedHash, 0, HASH_SIZE);
        return 0;
    }
    
    /* Use constant-time comparison to prevent timing attacks */
    result = constantTimeCompare(derivedHash, user->hash, HASH_SIZE);
    
    /* Securely clear sensitive data from memory */
    memset(derivedHash, 0, HASH_SIZE);
    
    return result;
}

/* Check if user exists */
int userExists(UserDatabase* db, const char* username) {
    if (db == NULL || username == NULL) {
        return 0;
    }
    
    if (!validateUsername(username)) {
        return 0;
    }
    
    return findUser(db, username) != NULL;
}

int main(void) {
    UserDatabase db;
    int result1, result2, result3, result4, result5;
    
    initDatabase(&db);
    
    /* Test case 1: Register valid user */
    printf("Test 1 - Register user 'alice_01': ");
    result1 = registerUser(&db, "alice_01", "SecurePass123!");
    printf("%s\\n", result1 ? "PASS" : "FAIL");
    
    /* Test case 2: Authenticate with correct password */
    printf("Test 2 - Authenticate alice_01 with correct password: ");
    result2 = authenticate(&db, "alice_01", "SecurePass123!");
    printf("%s\\n", result2 ? "PASS" : "FAIL");
    
    /* Test case 3: Authenticate with incorrect password */
    printf("Test 3 - Authenticate alice_01 with wrong password: ");
    result3 = !authenticate(&db, "alice_01", "WrongPass123!");
    printf("%s\\n", result3 ? "PASS" : "FAIL");
    
    /* Test case 4: Reject weak password */
    printf("Test 4 - Reject weak password '12345': ");
    result4 = !registerUser(&db, "bob_user", "12345");
    printf("%s\\n", result4 ? "PASS" : "FAIL");
    
    /* Test case 5: Reject duplicate username */
    printf("Test 5 - Reject duplicate username: ");
    result5 = !registerUser(&db, "alice_01", "AnotherPass456!");
    printf("%s\\n", result5 ? "PASS" : "FAIL");
    
    return 0;
}
