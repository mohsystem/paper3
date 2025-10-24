
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/* OpenSSL for cryptographic operations */
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <openssl/crypto.h>

/* Maximum limits for security */
#define MAX_SESSION_ID_LEN 256
#define MAX_USER_ID_LEN 256
#define MAX_IP_LEN 64
#define MAX_HASH_LEN 128
#define SESSION_ID_BYTES 32
#define HMAC_KEY_SIZE 32
#define MAX_SESSIONS 1024

/**
 * Secure user session management system in C.
 * Uses OpenSSL for cryptographic operations.
 * Prevents session fixation, hijacking, and timing attacks.
 */

/* Session structure with security attributes */
typedef struct {
    char session_id[MAX_SESSION_ID_LEN];
    char user_id[MAX_USER_ID_LEN];
    time_t created_at;
    time_t last_activity;
    char ip_address[MAX_IP_LEN];
    char user_agent_hash[MAX_HASH_LEN];
    int is_active;
} Session;

/* Session manager structure */
typedef struct {
    Session sessions[MAX_SESSIONS];
    int session_timeout;
    int absolute_timeout;
    unsigned char hmac_key[HMAC_KEY_SIZE];
} SessionManager;

/**
 * Initialize session manager with security parameters.
 * Returns 0 on success, -1 on failure.
 */
int session_manager_init(SessionManager* manager, int session_timeout, int absolute_timeout) {
    if (manager == NULL) {
        return -1;
    }
    
    /* Input validation: ensure timeouts are positive */
    if (session_timeout <= 0 || absolute_timeout <= 0) {
        return -1;
    }
    
    /* Initialize all sessions as inactive */
    memset(manager, 0, sizeof(SessionManager));
    manager->session_timeout = session_timeout;
    manager->absolute_timeout = absolute_timeout;
    
    /* Generate secure HMAC key using OpenSSL's cryptographically secure RNG */\n    if (RAND_bytes(manager->hmac_key, HMAC_KEY_SIZE) != 1) {\n        return -1;\n    }\n    \n    return 0;\n}\n\n/**\n * Hash a string using SHA-256.\n * Returns 0 on success, -1 on failure.\n */\nint hash_sha256(const char* input, size_t input_len, char* output, size_t output_size) {\n    if (input == NULL || output == NULL || output_size < (SHA256_DIGEST_LENGTH * 2 + 1)) {\n        return -1;\n    }\n    \n    unsigned char hash[SHA256_DIGEST_LENGTH];\n    \n    /* Use OpenSSL's SHA256 function */
    if (SHA256((const unsigned char*)input, input_len, hash) == NULL) {
        return -1;
    }
    
    /* Convert to hex string */
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        int written = snprintf(output + (i * 2), 3, "%02x", hash[i]);
        if (written != 2) {
            return -1;
        }
    }
    output[SHA256_DIGEST_LENGTH * 2] = '\\0';
    
    return 0;
}

/**
 * Constant-time string comparison to prevent timing attacks.
 * Uses CRYPTO_memcmp from OpenSSL which is constant-time.
 */
int constant_time_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return 0;
    }
    
    size_t a_len = strlen(a);
    size_t b_len = strlen(b);
    
    /* Strings must be same length for constant-time comparison */
    if (a_len != b_len) {
        return 0;
    }
    
    /* CRYPTO_memcmp returns 0 if equal (constant-time) */
    return CRYPTO_memcmp(a, b, a_len) == 0;
}

/**
 * Encode bytes to Base64 URL-safe format.
 * Returns 0 on success, -1 on failure.
 */
int base64_url_encode(const unsigned char* buffer, size_t length, char* output, size_t output_size) {
    if (buffer == NULL || output == NULL) {
        return -1;
    }
    
    /* Calculate required buffer size */
    size_t encoded_length = ((length + 2) / 3) * 4;
    if (output_size < encoded_length + 1) {
        return -1;
    }
    
    /* Use OpenSSL's EVP_EncodeBlock for Base64 encoding */\n    int result = EVP_EncodeBlock((unsigned char*)output, buffer, (int)length);\n    if (result < 0) {\n        return -1;\n    }\n    \n    /* Convert to URL-safe format: replace + with -, / with _ */\n    for (size_t i = 0; i < encoded_length && output[i] != '\\0'; i++) {\n        if (output[i] == '+') {\n            output[i] = '-';\n        } else if (output[i] == '/') {\n            output[i] = '_';\n        } else if (output[i] == '=') {\n            /* Remove padding */\n            output[i] = '\\0';\n            break;\n        }\n    }\n    \n    return 0;\n}\n\n/**\n * Create a new session with cryptographically secure ID.\n * Returns pointer to session_id on success, NULL on failure.\n */\nconst char* create_session(SessionManager* manager, const char* user_id, \n                          const char* ip_address, const char* user_agent) {\n    if (manager == NULL || user_id == NULL || ip_address == NULL || user_agent == NULL) {\n        return NULL;\n    }\n    \n    /* Input validation: ensure parameters are non-empty */\n    if (strlen(user_id) == 0 || strlen(user_id) > MAX_USER_ID_LEN - 1) {\n        return NULL;\n    }\n    if (strlen(ip_address) == 0 || strlen(ip_address) > MAX_IP_LEN - 1) {\n        return NULL;\n    }\n    if (strlen(user_agent) == 0) {\n        return NULL;\n    }\n    \n    /* Find available session slot */\n    int slot = -1;\n    for (int i = 0; i < MAX_SESSIONS; i++) {\n        if (!manager->sessions[i].is_active) {\n            slot = i;\n            break;\n        }\n    }\n    \n    if (slot == -1) {\n        return NULL;  /* No available slots */\n    }\n    \n    Session* session = &manager->sessions[slot];\n    \n    /* Generate cryptographically secure session ID (32 bytes = 256 bits) */\n    unsigned char random_bytes[SESSION_ID_BYTES];\n    if (RAND_bytes(random_bytes, SESSION_ID_BYTES) != 1) {\n        return NULL;\n    }\n    \n    /* Encode to Base64 URL-safe format */\n    if (base64_url_encode(random_bytes, SESSION_ID_BYTES, \n                         session->session_id, MAX_SESSION_ID_LEN) != 0) {\n        return NULL;\n    }\n    \n    /* Copy user_id with bounds checking */\n    size_t user_id_len = strlen(user_id);\n    if (user_id_len >= MAX_USER_ID_LEN) {\n        return NULL;\n    }\n    memcpy(session->user_id, user_id, user_id_len);\n    session->user_id[user_id_len] = '\\0';\n    \n    /* Copy ip_address with bounds checking */\n    size_t ip_len = strlen(ip_address);\n    if (ip_len >= MAX_IP_LEN) {\n        return NULL;\n    }\n    memcpy(session->ip_address, ip_address, ip_len);\n    session->ip_address[ip_len] = '\\0';\n    \n    /* Hash user agent to avoid storing sensitive browser fingerprint */\n    if (hash_sha256(user_agent, strlen(user_agent), \n                   session->user_agent_hash, MAX_HASH_LEN) != 0) {\n        return NULL;\n    }\n    \n    /* Set timestamps */\n    session->created_at = time(NULL);\n    session->last_activity = session->created_at;\n    session->is_active = 1;\n    \n    return session->session_id;\n}\n\n/**\n * Validate session and check for hijacking attempts.\n * Returns pointer to user_id on success, NULL on failure.\n */\nconst char* validate_session(SessionManager* manager, const char* session_id,\n                            const char* ip_address, const char* user_agent) {\n    if (manager == NULL || session_id == NULL || ip_address == NULL || user_agent == NULL) {\n        return NULL;\n    }\n    \n    /* Input validation: ensure parameters are non-empty */\n    if (strlen(session_id) == 0 || strlen(session_id) > MAX_SESSION_ID_LEN - 1) {\n        return NULL;\n    }\n    if (strlen(ip_address) == 0) {\n        return NULL;\n    }\n    if (strlen(user_agent) == 0) {\n        return NULL;\n    }\n    \n    /* Find session */\n    Session* session = NULL;\n    for (int i = 0; i < MAX_SESSIONS; i++) {\n        if (manager->sessions[i].is_active && \n            strcmp(manager->sessions[i].session_id, session_id) == 0) {\n            session = &manager->sessions[i];\n            break;\n        }\n    }\n    \n    if (session == NULL) {\n        return NULL;  /* Session not found */\n    }\n    \n    time_t current_time = time(NULL);\n    \n    /* Check absolute timeout (maximum session lifetime) */\n    if (current_time - session->created_at > manager->absolute_timeout) {\n        session->is_active = 0;\n        return NULL;\n    }\n    \n    /* Check idle timeout */\n    if (current_time - session->last_activity > manager->session_timeout) {\n        session->is_active = 0;\n        return NULL;\n    }\n    \n    /* Verify IP address matches (prevents session hijacking) */\n    /* Use constant-time comparison to prevent timing attacks */\n    if (!constant_time_equals(session->ip_address, ip_address)) {\n        session->is_active = 0;\n        return NULL;\n    }\n    \n    /* Hash current user agent and verify it matches */\n    char user_agent_hash[MAX_HASH_LEN];\n    if (hash_sha256(user_agent, strlen(user_agent), user_agent_hash, MAX_HASH_LEN) != 0) {\n        return NULL;\n    }\n    \n    /* Use constant-time comparison for user agent hash */\n    if (!constant_time_equals(session->user_agent_hash, user_agent_hash)) {\n        session->is_active = 0;\n        return NULL;\n    }\n    \n    /* Update last activity timestamp (session still valid) */\n    session->last_activity = current_time;\n    \n    return session->user_id;\n}\n\n/**\n * Securely destroy a session.\n * Returns 1 if destroyed, 0 if not found.\n */\nint destroy_session(SessionManager* manager, const char* session_id) {\n    if (manager == NULL || session_id == NULL) {\n        return 0;\n    }\n    \n    /* Input validation */\n    if (strlen(session_id) == 0) {\n        return 0;\n    }\n    \n    /* Find and destroy session */\n    for (int i = 0; i < MAX_SESSIONS; i++) {\n        if (manager->sessions[i].is_active && \n            strcmp(manager->sessions[i].session_id, session_id) == 0) {\n            \n            /* Clear sensitive data using memset_s if available */\n            #ifdef __STDC_LIB_EXT1__\n            memset_s(&manager->sessions[i], sizeof(Session), 0, sizeof(Session));\n            #else\n            /* Fallback: use volatile to prevent optimization */\n            volatile unsigned char* p = (volatile unsigned char*)&manager->sessions[i];\n            for (size_t j = 0; j < sizeof(Session); j++) {\n                p[j] = 0;\n            }\n            #endif\n            \n            manager->sessions[i].is_active = 0;\n            return 1;\n        }\n    }\n    \n    return 0;\n}\n\n/**\n * Cleanup session manager and clear sensitive data.\n */\nvoid session_manager_cleanup(SessionManager* manager) {\n    if (manager == NULL) {\n        return;\n    }\n    \n    /* Clear HMAC key using memset_s if available */\n    #ifdef __STDC_LIB_EXT1__\n    memset_s(manager->hmac_key, HMAC_KEY_SIZE, 0, HMAC_KEY_SIZE);\n    memset_s(manager->sessions, sizeof(manager->sessions), 0, sizeof(manager->sessions));\n    #else\n    /* Fallback: use volatile to prevent optimization */\n    volatile unsigned char* key_p = (volatile unsigned char*)manager->hmac_key;\n    for (size_t i = 0; i < HMAC_KEY_SIZE; i++) {\n        key_p[i] = 0;\n    }\n    volatile unsigned char* sess_p = (volatile unsigned char*)manager->sessions;\n    for (size_t i = 0; i < sizeof(manager->sessions); i++) {\n        sess_p[i] = 0;\n    }\n    #endif\n}\n\n/**\n * Test cases demonstrating secure session management.\n */\nint main(void) {\n    printf("=== Secure Session Manager Test Cases ===\\n\\n");\n    \n    SessionManager manager;\n    \n    /* Initialize session manager with 5-minute idle timeout, 1-hour absolute timeout */\n    if (session_manager_init(&manager, 300, 3600) != 0) {\n        fprintf(stderr, "Failed to initialize session manager\\n");\n        return 1;\n    }\n    \n    /* Test Case 1: Create and validate a valid session */\n    printf("Test 1: Create and validate valid session\\n");\n    const char* session_id = create_session(&manager, "user123", "192.168.1.100",\n                                          "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");\n    if (session_id == NULL) {\n        fprintf(stderr, "  Failed to create session\\n");\n        session_manager_cleanup(&manager);\n        return 1;\n    }\n    printf("  Created session: %.16s...\\n", session_id);\n    \n    const char* user_id = validate_session(&manager, session_id, "192.168.1.100",\n                                          "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");\n    printf("  Validated user: %s\\n", user_id != NULL ? user_id : "null");\n    if (user_id == NULL || strcmp(user_id, "user123") != 0) {\n        fprintf(stderr, "  Session validation failed\\n");\n        session_manager_cleanup(&manager);\n        return 1;\n    }\n    printf("  ✓ Pass\\n\\n");\n    \n    /* Test Case 2: Reject session with mismatched IP (hijacking prevention) */\n    printf("Test 2: Reject session with different IP address\\n");\n    user_id = validate_session(&manager, session_id, "192.168.1.200",\n                              "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");\n    printf("  Validation result: %s\\n", user_id != NULL ? user_id : "null");\n    if (user_id != NULL) {\n        fprintf(stderr, "  Should reject mismatched IP\\n");\n        session_manager_cleanup(&manager);\n        return 1;\n    }\n    printf("  ✓ Pass (correctly rejected)\\n\\n");\n    \n    /* Test Case 3: Reject session with mismatched user agent */\n    printf("Test 3: Reject session with different user agent\\n");\n    const char* session_id2 = create_session(&manager, "user456", "10.0.0.50", "Chrome/120.0.0.0");\n    if (session_id2 == NULL) {\n        fprintf(stderr, "  Failed to create session\\n");\n        session_manager_cleanup(&manager);\n        return 1;\n    }\n    user_id = validate_session(&manager, session_id2, "10.0.0.50", "Firefox/', type='text')