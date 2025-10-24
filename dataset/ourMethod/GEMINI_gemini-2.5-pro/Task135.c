#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <assert.h>

// To compile: gcc your_file_name.c -o your_program_name -lssl -lcrypto
// e.g. gcc Task135.c -o task135 -lssl -lcrypto

#define TOKEN_BYTE_LENGTH 32
#define TOKEN_HEX_LENGTH (TOKEN_BYTE_LENGTH * 2 + 1)
#define SESSION_ID_MAX_LEN 64
#define MAX_SESSIONS 10

typedef struct {
    char sessionId[SESSION_ID_MAX_LEN];
    char token[TOKEN_HEX_LENGTH];
} CsrfSession;

typedef struct {
    CsrfSession sessions[MAX_SESSIONS];
    int count;
} SessionStore;

/**
 * Initializes the session store.
 */
void init_session_store(SessionStore* store) {
    store->count = 0;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        store->sessions[i].sessionId[0] = '\0';
        store->sessions[i].token[0] = '\0';
    }
}

/**
 * Helper to convert bytes to a hex string.
 * Ensures the output buffer is null-terminated.
 */
static int bytes_to_hex(const unsigned char* bytes, size_t len, char* hex_out, size_t out_len) {
    if (out_len < len * 2 + 1) {
        return -1; // Output buffer too small
    }
    for (size_t i = 0; i < len; ++i) {
        sprintf(hex_out + (i * 2), "%02x", bytes[i]);
    }
    hex_out[len * 2] = '\0';
    return 0;
}

/**
 * Generates a CSRF token for a given session ID and stores it.
 * Returns 0 on success, -1 on failure.
 */
int generate_token_for_session(SessionStore* store, const char* session_id, char* token_out, size_t token_out_len) {
    if (!store || !session_id || strlen(session_id) >= SESSION_ID_MAX_LEN || store->count >= MAX_SESSIONS) {
        return -1;
    }
    
    unsigned char token_bytes[TOKEN_BYTE_LENGTH];
    if (RAND_bytes(token_bytes, sizeof(token_bytes)) != 1) {
        fprintf(stderr, "Error: RAND_bytes failed.\n");
        return -1;
    }

    char token_hex[TOKEN_HEX_LENGTH];
    if (bytes_to_hex(token_bytes, TOKEN_BYTE_LENGTH, token_hex, sizeof(token_hex)) != 0) {
        fprintf(stderr, "Error: Failed to convert token to hex.\n");
        return -1;
    }

    int index = -1;
    for (int i = 0; i < store->count; ++i) {
        if (strncmp(store->sessions[i].sessionId, session_id, SESSION_ID_MAX_LEN) == 0) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        index = store->count++;
    }

    strncpy(store->sessions[index].sessionId, session_id, SESSION_ID_MAX_LEN - 1);
    store->sessions[index].sessionId[SESSION_ID_MAX_LEN - 1] = '\0';
    
    strncpy(store->sessions[index].token, token_hex, TOKEN_HEX_LENGTH - 1);
    store->sessions[index].token[TOKEN_HEX_LENGTH - 1] = '\0';

    if (token_out != NULL && token_out_len >= TOKEN_HEX_LENGTH) {
        strncpy(token_out, token_hex, token_out_len - 1);
        token_out[token_out_len - 1] = '\0';
    }
    
    return 0;
}

/**
 * Validates a submitted token against the one stored for the session.
 * Uses a constant-time comparison to prevent timing attacks.
 */
bool validate_token(const SessionStore* store, const char* session_id, const char* submitted_token) {
    if (!store || !session_id || !submitted_token || *session_id == '\0' || *submitted_token == '\0') {
        return false;
    }

    const char* stored_token = NULL;
    for (int i = 0; i < store->count; ++i) {
        if (strncmp(store->sessions[i].sessionId, session_id, SESSION_ID_MAX_LEN) == 0) {
            stored_token = store->sessions[i].token;
            break;
        }
    }

    if (!stored_token) {
        return false;
    }

    size_t stored_len = strlen(stored_token);
    size_t submitted_len = strlen(submitted_token);

    if (stored_len != submitted_len) {
        return false;
    }

    return CRYPTO_memcmp(stored_token, submitted_token, stored_len) == 0;
}

int main() {
    SessionStore store;
    init_session_store(&store);

    const char* session_id1 = "user_session_abc123";
    const char* session_id2 = "user_session_def456";
    char token_buffer[TOKEN_HEX_LENGTH];

    printf("--- Running CSRF Protection Test Cases ---\n");

    // Test Case 1: Valid token
    printf("\n--- Test Case 1: Valid Token ---\n");
    assert(generate_token_for_session(&store, session_id1, token_buffer, sizeof(token_buffer)) == 0);
    printf("Generated Token for %s: %s\n", session_id1, token_buffer);
    bool is_valid1 = validate_token(&store, session_id1, token_buffer);
    printf("Validation result: %s\n", is_valid1 ? "PASS" : "FAIL");
    assert(is_valid1);

    // Test Case 2: Invalid token
    printf("\n--- Test Case 2: Invalid Token ---\n");
    assert(generate_token_for_session(&store, session_id1, token_buffer, sizeof(token_buffer)) == 0);
    printf("Generated Token for %s: %s\n", session_id1, token_buffer);
    const char* wrong_token = "invalid_token_this_is_wrong_1234567890abcdef";
    bool is_valid2 = validate_token(&store, session_id1, wrong_token);
    printf("Validation with wrong token result: %s\n", !is_valid2 ? "PASS" : "FAIL");
    assert(!is_valid2);

    // Test Case 3: Missing token in session
    printf("\n--- Test Case 3: Missing Token in Session ---\n");
    bool is_valid3 = validate_token(&store, "non_existent_session", "some_token");
    printf("Validation for non-existent session: %s\n", !is_valid3 ? "PASS" : "FAIL");
    assert(!is_valid3);

    // Test Case 4: Null/empty submitted token
    printf("\n--- Test Case 4: Null/Empty Submitted Token ---\n");
    assert(generate_token_for_session(&store, session_id1, token_buffer, sizeof(token_buffer)) == 0);
    printf("Generated Token for %s: %s\n", session_id1, token_buffer);
    bool is_valid4a = validate_token(&store, session_id1, NULL);
    bool is_valid4b = validate_token(&store, session_id1, "");
    printf("Validation with null token: %s\n", !is_valid4a ? "PASS" : "FAIL");
    printf("Validation with empty token: %s\n", !is_valid4b ? "PASS" : "FAIL");
    assert(!is_valid4a);
    assert(!is_valid4b);

    // Test Case 5: Token from a different session
    printf("\n--- Test Case 5: Token from a Different Session ---\n");
    char token_for_session1[TOKEN_HEX_LENGTH];
    char token_for_session2[TOKEN_HEX_LENGTH];
    assert(generate_token_for_session(&store, session_id1, token_for_session1, sizeof(token_for_session1)) == 0);
    assert(generate_token_for_session(&store, session_id2, token_for_session2, sizeof(token_for_session2)) == 0);
    printf("Token for %s: %s\n", session_id1, token_for_session1);
    printf("Token for %s: %s\n", session_id2, token_for_session2);
    bool is_valid5 = validate_token(&store, session_id1, token_for_session2);
    printf("Validating session 1 with session 2's token: %s\n", !is_valid5 ? "PASS" : "FAIL");
    assert(!is_valid5);

    printf("\n--- All test cases completed ---\n");

    return 0;
}