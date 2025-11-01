
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

/* Constants for input validation */
#define MAX_USERNAME_LENGTH 50
#define MAX_PASSWORD_LENGTH 128
#define MAX_SESSION_LENGTH 256
#define MAX_QUERY_LENGTH 1024
#define SALT_SIZE 16
#define SESSION_TOKEN_SIZE 32
#define MAX_USERS 10
#define MAX_SESSIONS 10
#define BASE64_ENCODE_OUT_SIZE(s) (((s) + 2) / 3 * 4)

/* User data structure */
typedef struct {
    char username[MAX_USERNAME_LENGTH + 1];
    char passwordHash[256];
    char salt[256];
    bool isModerator;
    bool inUse;
} UserData;

/* Session data structure */
typedef struct {
    char username[MAX_USERNAME_LENGTH + 1];
    char sessionToken[512];
    bool inUse;
} SessionData;

/* Simulated database and session store */
static UserData userDatabase[MAX_USERS];
static SessionData sessionStore[MAX_SESSIONS];

/* Base64 encoding characters */
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Base64 encoding function */
void base64_encode(const unsigned char* input, size_t length, char* output) {
    size_t i = 0, j = 0;
    
    if (output == NULL || input == NULL) {
        return;
    }
    
    for (i = 0; i < length; i += 3) {
        unsigned int val = input[i] << 16;
        if (i + 1 < length) val |= input[i + 1] << 8;
        if (i + 2 < length) val |= input[i + 2];
        
        output[j++] = base64_chars[(val >> 18) & 0x3F];
        output[j++] = base64_chars[(val >> 12) & 0x3F];
        output[j++] = i + 1 < length ? base64_chars[(val >> 6) & 0x3F] : '=';
        output[j++] = i + 2 < length ? base64_chars[val & 0x3F] : '=';
    }
    output[j] = '\\0';
}

/* Base64 decoding table */
static const unsigned char base64_table[256] = {
    ['A'] = 0, ['B'] = 1, ['C'] = 2, ['D'] = 3, ['E'] = 4, ['F'] = 5,
    ['G'] = 6, ['H'] = 7, ['I'] = 8, ['J'] = 9, ['K'] = 10, ['L'] = 11,
    ['M'] = 12, ['N'] = 13, ['O'] = 14, ['P'] = 15, ['Q'] = 16, ['R'] = 17,
    ['S'] = 18, ['T'] = 19, ['U'] = 20, ['V'] = 21, ['W'] = 22, ['X'] = 23,
    ['Y'] = 24, ['Z'] = 25, ['a'] = 26, ['b'] = 27, ['c'] = 28, ['d'] = 29,
    ['e'] = 30, ['f'] = 31, ['g'] = 32, ['h'] = 33, ['i'] = 34, ['j'] = 35,
    ['k'] = 36, ['l'] = 37, ['m'] = 38, ['n'] = 39, ['o'] = 40, ['p'] = 41,
    ['q'] = 42, ['r'] = 43, ['s'] = 44, ['t'] = 45, ['u'] = 46, ['v'] = 47,
    ['w'] = 48, ['x'] = 49, ['y'] = 50, ['z'] = 51, ['0'] = 52, ['1'] = 53,
    ['2'] = 54, ['3'] = 55, ['4'] = 56, ['5'] = 57, ['6'] = 58, ['7'] = 59,
    ['8'] = 60, ['9'] = 61, ['+'] = 62, ['/'] = 63
};

/* Base64 decoding function */
size_t base64_decode(const char* input, unsigned char* output, size_t max_output) {
    size_t len = strlen(input);
    size_t i, j = 0;
    
    if (len == 0 || len % 4 != 0 || output == NULL) {
        return 0;
    }
    
    for (i = 0; i < len && j < max_output - 3; i += 4) {
        unsigned int val = (base64_table[(unsigned char)input[i]] << 18) |
                          (base64_table[(unsigned char)input[i + 1]] << 12) |
                          (base64_table[(unsigned char)input[i + 2]] << 6) |
                          base64_table[(unsigned char)input[i + 3]];
        
        output[j++] = (val >> 16) & 0xFF;
        if (input[i + 2] != '=') output[j++] = (val >> 8) & 0xFF;
        if (input[i + 3] != '=') output[j++] = val & 0xFF;
    }
    output[j] = '\\0';
    
    return j;
}

/* Generate cryptographically secure random bytes using OpenSSL */
bool generateSecureRandom(unsigned char* buffer, size_t length) {
    if (buffer == NULL || length == 0) {
        return false;
    }
    
    /* Use OpenSSL's CSPRNG */\n    if (RAND_bytes(buffer, (int)length) != 1) {\n        fprintf(stderr, "Failed to generate secure random bytes\\n");\n        return false;\n    }\n    \n    return true;\n}\n\n/* Secure password hashing with SHA-256 and salt */\nvoid hashPassword(const char* password, const char* salt, char* output) {\n    unsigned char saltBytes[256];\n    unsigned char combined[512];\n    unsigned char hash[SHA256_DIGEST_LENGTH];\n    size_t saltLen, passLen, combinedLen;\n    \n    if (password == NULL || salt == NULL || output == NULL) {\n        output[0] = '\\0';\n        return;\n    }\n    \n    /* Decode salt from base64 */\n    saltLen = base64_decode(salt, saltBytes, sizeof(saltBytes));\n    if (saltLen == 0) {\n        output[0] = '\\0';\n        return;\n    }\n    \n    /* Combine salt and password */\n    passLen = strlen(password);\n    if (saltLen + passLen >= sizeof(combined)) {\n        output[0] = '\\0';\n        return;\n    }\n    \n    memcpy(combined, saltBytes, saltLen);\n    memcpy(combined + saltLen, password, passLen);\n    combinedLen = saltLen + passLen;\n    \n    /* Hash with SHA-256 */\n    SHA256(combined, combinedLen, hash);\n    \n    /* Encode hash to base64 */\n    base64_encode(hash, SHA256_DIGEST_LENGTH, output);\n    \n    /* Clear sensitive data using memset_s or explicit zero */\n    memset(combined, 0, sizeof(combined));\n    memset(saltBytes, 0, sizeof(saltBytes));\n}\n\n/* Constant-time comparison to prevent timing attacks */\nbool constantTimeCompare(const char* a, const char* b) {\n    size_t len_a, len_b, i;\n    volatile unsigned char result = 0;\n    \n    if (a == NULL || b == NULL) {\n        return false;\n    }\n    \n    len_a = strlen(a);\n    len_b = strlen(b);\n    \n    if (len_a != len_b) {\n        return false;\n    }\n    \n    for (i = 0; i < len_a; i++) {\n        result |= a[i] ^ b[i];\n    }\n    \n    return result == 0;\n}\n\n/* Create secure session token */\nbool createSession(const char* username, char* sessionToken) {\n    unsigned char randomBytes[SESSION_TOKEN_SIZE];\n    int i;\n    \n    if (username == NULL || sessionToken == NULL) {\n        return false;\n    }\n    \n    /* Generate secure random token */\n    if (!generateSecureRandom(randomBytes, SESSION_TOKEN_SIZE)) {\n        return false;\n    }\n    \n    /* Encode to base64 */\n    base64_encode(randomBytes, SESSION_TOKEN_SIZE, sessionToken);\n    \n    /* Store in session store */\n    for (i = 0; i < MAX_SESSIONS; i++) {\n        if (!sessionStore[i].inUse) {\n            strncpy(sessionStore[i].username, username, MAX_USERNAME_LENGTH);\n            sessionStore[i].username[MAX_USERNAME_LENGTH] = '\\0';\n            strncpy(sessionStore[i].sessionToken, sessionToken, sizeof(sessionStore[i].sessionToken) - 1);\n            sessionStore[i].sessionToken[sizeof(sessionStore[i].sessionToken) - 1] = '\\0';\n            sessionStore[i].inUse = true;\n            break;\n        }\n    }\n    \n    /* Clear sensitive data */\n    memset(randomBytes, 0, sizeof(randomBytes));\n    \n    return true;\n}\n\n/* Validate session token */\nbool validateSession(const char* username, const char* sessionToken) {\n    int i;\n    \n    if (username == NULL || sessionToken == NULL) {\n        return false;\n    }\n    \n    if (strlen(username) == 0 || strlen(sessionToken) == 0) {\n        return false;\n    }\n    \n    /* Find session in store */\n    for (i = 0; i < MAX_SESSIONS; i++) {\n        if (sessionStore[i].inUse && \n            strcmp(sessionStore[i].username, username) == 0) {\n            /* Constant-time comparison */\n            return constantTimeCompare(sessionStore[i].sessionToken, sessionToken);\n        }\n    }\n    \n    return false;\n}\n\n/* Add user with secure password hashing */\nbool addUser(const char* username, const char* password, bool isModerator) {\n    unsigned char saltBytes[SALT_SIZE];\n    char salt[256];\n    int i;\n    \n    if (username == NULL || password == NULL) {\n        return false;\n    }\n    \n    /* Generate cryptographically secure salt */\n    if (!generateSecureRandom(saltBytes, SALT_SIZE)) {\n        return false;\n    }\n    \n    /* Encode salt to base64 */\n    base64_encode(saltBytes, SALT_SIZE, salt);\n    \n    /* Find empty slot in database */\n    for (i = 0; i < MAX_USERS; i++) {\n        if (!userDatabase[i].inUse) {\n            strncpy(userDatabase[i].username, username, MAX_USERNAME_LENGTH);\n            userDatabase[i].username[MAX_USERNAME_LENGTH] = '\\0';\n            \n            strncpy(userDatabase[i].salt, salt, sizeof(userDatabase[i].salt) - 1);\n            userDatabase[i].salt[sizeof(userDatabase[i].salt) - 1] = '\\0';\n            \n            hashPassword(password, salt, userDatabase[i].passwordHash);\n            \n            userDatabase[i].isModerator = isModerator;\n            userDatabase[i].inUse = true;\n            \n            /* Clear sensitive data */\n            memset(saltBytes, 0, sizeof(saltBytes));\n            \n            return true;\n        }\n    }\n    \n    /* Clear sensitive data */\n    memset(saltBytes, 0, sizeof(saltBytes));\n    \n    return false;\n}\n\n/* Validate username (alphanumeric + underscore only) */\nbool isValidUsername(const char* username) {\n    size_t i, len;\n    \n    if (username == NULL) {\n        return false;\n    }\n    \n    len = strlen(username);\n    if (len == 0 || len > MAX_USERNAME_LENGTH) {\n        return false;\n    }\n    \n    for (i = 0; i < len; i++) {\n        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {\n            return false;\n        }\n    }\n    \n    return true;\n}\n\n/* URL decode helper */\nvoid urlDecode(const char* encoded, char* decoded, size_t max_decoded) {\n    size_t i = 0, j = 0;\n    size_t len = strlen(encoded);\n    \n    if (encoded == NULL || decoded == NULL) {\n        return;\n    }\n    \n    while (i < len && j < max_decoded - 1) {\n        if (encoded[i] == '%' && i + 2 < len) {\n            int value;\n            if (sscanf(encoded + i + 1, "%2x", &value) == 1) {\n                decoded[j++] = (char)value;\n                i += 3;\n            } else {\n                decoded[j++] = encoded[i++];\n            }\n        } else if (encoded[i] == '+') {\n            decoded[j++] = ' ';\n            i++;\n        } else {\n            decoded[j++] = encoded[i++];\n        }\n    }\n    decoded[j] = '\\0';\n}\n\n/* Parse query string parameter */\nbool getQueryParam(const char* query, const char* param, char* value, size_t max_value) {\n    const char* start;\n    const char* end;\n    size_t paramLen;\n    size_t valueLen;\n    \n    if (query == NULL || param == NULL || value == NULL) {\n        return false;\n    }\n    \n    paramLen = strlen(param);\n    start = strstr(query, param);\n    \n    while (start != NULL) {\n        /* Check if this is the parameter we want (not a substring) */\n        if ((start == query || start[-1] == '&') && start[paramLen] == '=') {\n            start += paramLen + 1;\n            end = strchr(start, '&');\n            valueLen = end ? (size_t)(end - start) : strlen(start);\n            \n            if (valueLen >= max_value) {\n                return false;\n            }\n            \n            strncpy(value, start, valueLen);\n            value[valueLen] = '\\0';
            return true;
        }
        
        start = strstr(start + 1, param);
    }
    
    return false;
}

/**
 * Authenticates user and checks if they are a moderator
 * @param username - validated username (max 50 chars, alphanumeric + underscore)
 * @param password - password for authentication
 * @param session - session token for validation
 * @return true if user is authenticated and is a moderator, false otherwise
 */
bool auth_mod(const char* username, const char* password, const char* session) {
    int i;
    char providedHash[256];
    
    /* Input validation - reject NULL or empty inputs */
    if (username == NULL || password == NULL || session == NULL) {
        return false;
    }
    
    if (strlen(username) == 0 || strlen(password) == 0 || strlen(session) == 0) {
        return false;
    }
    
    /* Input validation - length checks */
    if (strlen(username) > MAX_USERNAME_LENGTH || 
        strlen(password) > MAX_PASSWORD_LENGTH || 
        strlen(session) > MAX_SESSION_LENGTH) {
        return false;
    }
    
    /* Input validation - sanitize username */
    if (!isValidUsername(username)) {
        return false;
    }
    
    /* Check if user exists */
    for (i = 0; i < MAX_USERS; i++) {
        if (userDatabase[i].inUse && 
            strcmp(userDatabase[i].username, username) == 0) {
            
            /* Validate session token */
            if (!validateSession(username, session)) {
                return false;
            }
            
            /* Verify password with constant-time comparison */
            hashPassword(password, userDatabase[i].salt, providedHash);
            
            if (!constantTimeCompare(userDatabase[i].passwordHash, providedHash)) {
                /* Clear sensitive data */
                memset(providedHash, 0, sizeof(providedHash));
                return false;
            }
            
            /* Clear sensitive data */
            memset(providedHash, 0, sizeof(providedHash));
            
            /* Return moderator status */
            return userDatabase[i].isModerator;
        }
    }
    
    return false;
}

/**
 * Handles /check_mod endpoint
 * Extracts username, password, and session from URL parameters
 * Calls auth_mod() to verify moderator status
 */
bool check_mod(const char* queryString) {
    char username[MAX_USERNAME_LENGTH + 1];
    char password[MAX_PASSWORD_LENGTH + 1];
    char session[MAX_SESSION_LENGTH + 1];
    char decodedUsername[MAX_USERNAME_LENGTH + 1];
    char decodedPassword[MAX_PASSWORD_LENGTH + 1];
    char decodedSession[MAX_SESSION_LENGTH + 1];
    
    /* Input validation - reject NULL or excessively long query strings */
    if (queryString == NULL || strlen(queryString) > MAX_QUERY_LENGTH) {
        return false;
    }
    
    /* Extract parameters */
    if (!getQueryParam(queryString, "username", username, sizeof(username)) ||
        !getQueryParam(queryString, "password", password, sizeof(password)) ||
        !getQueryParam(queryString, "session", session, sizeof(session))) {
        return false;
    }
    
    /* URL decode parameters */
    urlDecode(username, decodedUsername, sizeof(decodedUsername));
    urlDecode(password, decodedPassword, sizeof(decodedPassword));
    urlDecode(session, decodedSession, sizeof(decodedSession));
    
    /* Call authentication function */
    return auth_mod(decodedUsername, decodedPassword, decodedSession);
}

int main(void) {
    char adminSession[512];
    char modSession[512];
    char user1Session[512];
    char user2Session[512];
    char guestSession[512];
    char query1[MAX_QUERY_LENGTH];
    char query2[MAX_QUERY_LENGTH];
    char query3[MAX_QUERY_LENGTH];
    char query4[MAX_QUERY_LENGTH];
    char query5[MAX_QUERY_LENGTH];
    bool result1, result2, result3, result4, result5;
    
    /* Initialize database structures */
    memset(userDatabase, 0, sizeof(userDatabase));
    memset(sessionStore, 0, sizeof(sessionStore));
    
    /* Initialize OpenSSL */
    OpenSSL_add_all_algorithms();
    
    /* Initialize test database with secure password hashing */
    addUser("admin", "AdminPass123!", true);
    addUser("moderator", "ModPass456!", true);
    addUser("user1", "UserPass789!", false);
    addUser("user2", "Pass2024Secure!", false);
    addUser("guest", "GuestPass321!", false);
    
    /* Create sessions for test users */
    createSession("admin", adminSession);
    createSession("moderator", modSession);
    createSession("user1", user1Session);
    createSession("user2", user2Session);
    createSession("guest", guestSession);
    
    printf("=== Test Cases for Moderator Check ===\\n\\n");
    
    /* Test Case 1: Valid moderator (admin) */
    snprintf(query1, sizeof(query1), "username=admin&password=AdminPass123!&session=%s", adminSession);
    result1 = check_mod(query1);
    printf("Test 1 - Valid moderator (admin): %s\\n", result1 ? "PASS" : "FAIL");
    printf("Expected: true, Got: %s\\n\\n", result1 ? "true" : "false");
    
    /* Test Case 2: Valid moderator (moderator user) */
    snprintf(query2, sizeof(query2), "username=moderator&password=ModPass456!&session=%s", modSession);
    result2 = check_mod(query2);
    printf("Test 2 - Valid moderator (moderator): %s\\n", result2 ? "PASS" : "FAIL");
    printf("Expected: true, Got: %s\\n\\n", result2 ? "true" : "false");
    
    /* Test Case 3: Valid user but not moderator */
    snprintf(query3, sizeof(query3), "username=user1&password=UserPass789!&session=%s", user1Session);
    result3 = check_mod(query3);
    printf("Test 3 - Valid user but not moderator: %s\\n", !result3 ? "PASS" : "FAIL");
    printf("Expected: false, Got: %s\\n\\n", result3 ? "true" : "false");
    
    /* Test Case 4: Invalid credentials */
    snprintf(query4, sizeof(query4), "username=admin&password=WrongPassword&session=%s", adminSession);
    result4 = check_mod(query4);
    printf("Test 4 - Invalid credentials: %s\\n", !result4 ? "PASS" : "FAIL");
    printf("Expected: false, Got: %s\\n\\n", result4 ? "true" : "false");
    
    /* Test Case 5: Invalid session token */
    snprintf(query5, sizeof(query5), "username=admin&password=AdminPass123!&session=InvalidSession123");
    result5 = check_mod(query5);
    printf("Test 5 - Invalid session token: %s\\n", !result5 ? "PASS" : "FAIL");
    printf("Expected: false, Got: %s\\n\\n", result5 ? "true" : "false");
    
    /* Cleanup OpenSSL */
    EVP_cleanup();
    
    return 0;
}
