
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

/* Security constants */
#define MIN_PASSWORD_LENGTH 8
#define SALT_SIZE 16
#define PBKDF2_ITERATIONS 210000
#define HASH_SIZE 32
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 128

/**
 * Generates cryptographically secure random salt
 * Uses OpenSSL RAND_bytes for CSPRNG (CWE-330, CWE-759)
 */
int generate_salt(unsigned char* salt, int size) {
    if (salt == NULL || size <= 0) {
        return 0;
    }
    /* RAND_bytes provides cryptographically secure randomness */
    int result = RAND_bytes(salt, size);
    return (result == 1);
}

/**
 * Hashes password using PBKDF2-HMAC-SHA256
 * Uses high iteration count and unique salt (CWE-759, CWE-916)
 */
int hash_password(const char* password, const unsigned char* salt, 
                  int salt_len, unsigned char* hash, int hash_len) {
    if (password == NULL || salt == NULL || hash == NULL || 
        salt_len <= 0 || hash_len <= 0) {
        return 0;
    }
    
    /* PBKDF2-HMAC-SHA256 for secure password hashing */
    int result = PKCS5_PBKDF2_HMAC(
        password,
        (int)strlen(password),
        salt,
        salt_len,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        hash_len,
        hash
    );
    
    return (result == 1);
}

/**
 * Validates username format
 * Prevents injection attacks (CWE-20)
 */
int validate_username(const char* username) {
    if (username == NULL) {
        return 0;
    }
    
    size_t len = strlen(username);
    if (len < 3 || len > MAX_USERNAME_LEN) {
        return 0;
    }
    
    /* Allow only alphanumeric and underscore */
    for (size_t i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return 0;
        }
    }
    
    return 1;
}

/**
 * Validates password meets security requirements
 * Enforces strong password policy (CWE-521)
 */
int validate_password(const char* password) {
    if (password == NULL) {
        return 0;
    }
    
    size_t len = strlen(password);
    if (len < MIN_PASSWORD_LENGTH || len > MAX_PASSWORD_LEN) {
        return 0;
    }
    
    int has_upper = 0, has_lower = 0, has_digit = 0, has_special = 0;
    
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) has_upper = 1;
        else if (islower(c)) has_lower = 1;
        else if (isdigit(c)) has_digit = 1;
        else if (ispunct(c)) has_special = 1;
    }
    
    return (has_upper && has_lower && has_digit && has_special);
}

/**
 * Base64 encoding helper
 */
char* base64_encode(const unsigned char* data, int len) {
    if (data == NULL || len <= 0) {
        return NULL;
    }
    
    static const char base64_chars[] = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    int encoded_len = 4 * ((len + 2) / 3);
    char* encoded = (char*)malloc(encoded_len + 1);
    if (encoded == NULL) {
        return NULL;
    }
    
    int i = 0, j = 0;
    unsigned char array3[3];
    unsigned char array4[4];
    int k = 0;
    
    while (len--) {
        array3[i++] = *(data++);
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for(i = 0; i < 4; i++) {
                encoded[j++] = base64_chars[array4[i]];
            }
            i = 0;
        }
    }
    
    if (i) {
        for(k = i; k < 3; k++) {
            array3[k] = '\\0';
        }
        
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (k = 0; k < i + 1; k++) {
            encoded[j++] = base64_chars[array4[k]];
        }
        
        while(i++ < 3) {
            encoded[j++] = '=';
        }
    }
    
    encoded[j] = '\\0';
    return encoded;
}

/**
 * Registers a user with secure password storage
 * Returns NULL on error, allocated string on success (caller must free)
 */
char* register_user(const char* username, const char* password) {
    /* Input validation - reject invalid input early (CWE-20) */
    if (!validate_username(username)) {
        char* error = strdup("Error: Invalid username format");
        return error;
    }
    
    if (!validate_password(password)) {
        char* error = strdup("Error: Password must be at least 8 characters with uppercase, lowercase, digit, and special character");
        return error;
    }
    
    /* Allocate buffers */
    unsigned char salt[SALT_SIZE];
    unsigned char password_hash[HASH_SIZE];
    unsigned char salt_and_hash[SALT_SIZE + HASH_SIZE];
    
    /* Initialize to zero */
    memset(salt, 0, SALT_SIZE);
    memset(password_hash, 0, HASH_SIZE);
    memset(salt_and_hash, 0, SALT_SIZE + HASH_SIZE);
    
    /* Generate cryptographically secure salt (CWE-330, CWE-759) */
    if (!generate_salt(salt, SALT_SIZE)) {
        memset(salt, 0, SALT_SIZE);
        char* error = strdup("Error: Registration failed");
        return error;
    }
    
    /* Hash password with PBKDF2-HMAC-SHA256 (CWE-916) */
    if (!hash_password(password, salt, SALT_SIZE, password_hash, HASH_SIZE)) {
        memset(salt, 0, SALT_SIZE);
        memset(password_hash, 0, HASH_SIZE);
        char* error = strdup("Error: Registration failed");
        return error;
    }
    
    /* Concatenate salt and hash */
    memcpy(salt_and_hash, salt, SALT_SIZE);
    memcpy(salt_and_hash + SALT_SIZE, password_hash, HASH_SIZE);
    
    /* Encode to Base64 */
    char* stored_value = base64_encode(salt_and_hash, SALT_SIZE + HASH_SIZE);
    
    /* Clear sensitive data from memory (CWE-244) */
    memset(salt, 0, SALT_SIZE);
    memset(password_hash, 0, HASH_SIZE);
    memset(salt_and_hash, 0, SALT_SIZE + HASH_SIZE);
    
    /* In production: store username and stored_value in database */
    /* Never log passwords or hashes (CWE-532) */
    
    return stored_value;
}

int main(void) {
    char* result = NULL;
    
    /* Test case 1: Valid registration */
    printf("Test 1 - Valid user:\\n");
    result = register_user("testUser123", "SecurePass123!");
    if (result != NULL) {
        printf("%s\\n", strncmp(result, "Error", 5) == 0 ? result : "Success: Hash stored");
        free(result);
        result = NULL;
    }
    
    /* Test case 2: Weak password */
    printf("\\nTest 2 - Weak password:\\n");
    result = register_user("user456", "WeakPass123");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 3: Invalid username */
    printf("\\nTest 3 - Invalid username:\\n");
    result = register_user("ab", "ValidPass123!");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 4: Password too short */
    printf("\\nTest 4 - Short password:\\n");
    result = register_user("validUser", "Sh0rt!");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
        result = NULL;
    }
    
    /* Test case 5: Another valid registration */
    printf("\\nTest 5 - Another valid user:\\n");
    result = register_user("anotherUser_99", "AnotherSecure99#");
    if (result != NULL) {
        printf("%s\\n", strncmp(result, "Error", 5) == 0 ? result : "Success: Hash stored");
        free(result);
        result = NULL;
    }
    
    return 0;
}
