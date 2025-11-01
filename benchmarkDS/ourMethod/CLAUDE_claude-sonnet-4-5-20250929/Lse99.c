
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <libpq-fe.h>

/* Password policy constants */
#define MIN_PASSWORD_LENGTH 12
#define MAX_PASSWORD_LENGTH 128
#define MAX_USERNAME_LENGTH 50
#define PBKDF2_ITERATIONS 210000
#define SALT_LENGTH 16
#define HASH_LENGTH 32

/* Base64 encoding table */
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

/**
 * Base64 encode function
 * Allocates memory that caller must free
 */
char* base64_encode(const unsigned char* data, size_t len) {
    size_t output_length = 4 * ((len + 2) / 3);
    char* encoded = malloc(output_length + 1);
    if (!encoded) return NULL;
    
    size_t i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    size_t pos = 0;
    
    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                encoded[pos++] = base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++)
            char_array_3[j] = '\\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++)
            encoded[pos++] = base64_chars[char_array_4[j]];
        
        while(i++ < 3)
            encoded[pos++] = '=';
    }
    
    encoded[pos] = '\\0';
    return encoded;
}

/**
 * Generates cryptographically secure random salt
 * Uses OpenSSL RAND_bytes for CSPRNG (CWE-330, CWE-759)
 */
int generate_salt(unsigned char* salt, int length) {
    if (RAND_bytes(salt, length) != 1) {
        return 0;
    }
    return 1;
}

/**
 * Hashes password using PBKDF2-HMAC-SHA256 with unique salt
 * Prevents rainbow table attacks (CWE-759)
 * Uses strong KDF (CWE-916)
 * Returns allocated string that caller must free
 */
char* hash_password(const char* password, const unsigned char* salt) {
    unsigned char hash[HASH_LENGTH];
    unsigned char combined[SALT_LENGTH + HASH_LENGTH];
    char* result = NULL;
    
    /* Use PBKDF2-HMAC-SHA256 */
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                          salt, SALT_LENGTH,
                          PBKDF2_ITERATIONS,
                          EVP_sha256(),
                          HASH_LENGTH, hash) != 1) {
        return NULL;
    }
    
    /* Combine salt and hash */
    memcpy(combined, salt, SALT_LENGTH);
    memcpy(combined + SALT_LENGTH, hash, HASH_LENGTH);
    
    /* Base64 encode */
    result = base64_encode(combined, SALT_LENGTH + HASH_LENGTH);
    
    /* Clear sensitive data using memset_s if available, otherwise explicit_bzero or volatile */
#ifdef __STDC_LIB_EXT1__
    memset_s(hash, HASH_LENGTH, 0, HASH_LENGTH);
#else
    OPENSSL_cleanse(hash, HASH_LENGTH);
#endif
    
    return result;
}

/**
 * Validates username to prevent injection (CWE-89)
 * Enforces length and character whitelist (CWE-20)
 */
int is_valid_username(const char* username) {
    size_t len;
    size_t i;
    
    if (!username) return 0;
    
    len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return 0;
    }
    
    /* Whitelist: alphanumeric, underscore, hyphen only */
    for (i = 0; i < len; i++) {
        char c = username[i];
        if (!isalnum(c) && c != '_' && c != '-') {
            return 0;
        }
    }
    
    return 1;
}

/**
 * Validates password meets security policy (CWE-521)
 */
int is_valid_password(const char* password) {
    size_t len;
    size_t i;
    int has_upper = 0, has_lower = 0;
    int has_digit = 0, has_special = 0;
    const char* special_chars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    if (!password) return 0;
    
    len = strlen(password);
    if (len < MIN_PASSWORD_LENGTH || len > MAX_PASSWORD_LENGTH) {
        return 0;
    }
    
    for (i = 0; i < len; i++) {
        char c = password[i];
        if (isupper(c)) has_upper = 1;
        if (islower(c)) has_lower = 1;
        if (isdigit(c)) has_digit = 1;
        if (strchr(special_chars, c)) has_special = 1;
    }
    
    return has_upper && has_lower && has_digit && has_special;
}

/**
 * Registers user with secure password hashing
 * Uses parameterized queries to prevent SQL injection (CWE-89)
 */
int register_user(const char* username, const char* password) {
    unsigned char salt[SALT_LENGTH];
    char* hashed_password = NULL;
    PGconn* conn = NULL;
    PGresult* res = NULL;
    const char* param_values[2];
    const char* conninfo;
    int success = 0;
    
    /* Input validation (CWE-20) */
    if (!is_valid_username(username)) {
        fprintf(stderr, "Invalid username format\\n");
        return 0;
    }
    
    if (!is_valid_password(password)) {
        fprintf(stderr, "Password does not meet security requirements\\n");
        return 0;
    }
    
    /* Generate unique salt (CWE-759, CWE-330) */
    if (!generate_salt(salt, SALT_LENGTH)) {
        fprintf(stderr, "Failed to generate salt\\n");
        return 0;
    }
    
    /* Hash password with salt (CWE-916) */
    hashed_password = hash_password(password, salt);
    if (!hashed_password) {
        fprintf(stderr, "Failed to hash password\\n");
        OPENSSL_cleanse(salt, SALT_LENGTH);
        return 0;
    }
    
    /* Database connection - should be from environment */
    conninfo = getenv("DB_CONNINFO");
    if (!conninfo) {
        conninfo = "host=localhost dbname=testdb user=testuser password=testpass";
    }
    
    conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        fprintf(stderr, "Database connection failed\\n");
        goto cleanup;
    }
    
    /* Use parameterized query to prevent SQL injection (CWE-89) */
    param_values[0] = username;
    param_values[1] = hashed_password;
    
    res = PQexecParams(conn,
        "INSERT INTO users (username, password_hash) VALUES ($1, $2)",
        2,
        NULL,
        param_values,
        NULL,
        NULL,
        0);
    
    if (PQresultStatus(res) == PGRES_COMMAND_OK) {
        success = 1;
    } else {
        fprintf(stderr, "Database operation failed\\n");
    }
    
cleanup:
    /* Clear sensitive data */
    OPENSSL_cleanse(salt, SALT_LENGTH);
    
    if (hashed_password) {
        memset(hashed_password, 0, strlen(hashed_password));
        free(hashed_password);
    }
    
    if (res) PQclear(res);
    if (conn) PQfinish(conn);
    
    return success;
}

int main(void) {
    /* Test cases */
    printf("Test 1: Valid user\\n");
    printf("%d\\n", register_user("testuser1", "SecureP@ssw0rd123"));
    
    printf("\\nTest 2: Valid user with special chars\\n");
    printf("%d\\n", register_user("john_doe", "MyP@ssw0rd!2024"));
    
    printf("\\nTest 3: Invalid username (too long)\\n");
    char long_username[52];
    memset(long_username, 'a', 51);
    long_username[51] = '\\0';
    printf("%d\\n", register_user(long_username, "ValidP@ss123"));
    
    printf("\\nTest 4: Invalid password (too short)\\n");
    printf("%d\\n", register_user("alice", "Short1!"));
    
    printf("\\nTest 5: Invalid username (SQL injection attempt)\\n");
    printf("%d\\n", register_user("user'; DROP TABLE users--", "SecureP@ssw0rd123"));
    
    return 0;
}
