
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

#define TOKEN_LENGTH 32
#define TOKEN_HEX_LENGTH (TOKEN_LENGTH * 2)
#define MAX_TOKENS 1000
#define TOKEN_VALIDITY_SECONDS 3600

/* Token entry structure */
typedef struct {
    char token[TOKEN_HEX_LENGTH + 1];
    int64_t expiration;
    int active;
} TokenEntry;

/* CSRF Protection context */
typedef struct {
    TokenEntry* tokens;
    size_t capacity;
    size_t count;
} CSRFContext;

/* Initialize CSRF context */
CSRFContext* csrf_init(void) {
    CSRFContext* ctx = (CSRFContext*)calloc(1, sizeof(CSRFContext));
    if (!ctx) {
        return NULL;
    }
    
    ctx->capacity = MAX_TOKENS;
    ctx->tokens = (TokenEntry*)calloc(MAX_TOKENS, sizeof(TokenEntry));
    if (!ctx->tokens) {
        free(ctx);
        return NULL;
    }
    
    ctx->count = 0;
    return ctx;
}

/* Free CSRF context and clear sensitive data */
void csrf_cleanup(CSRFContext* ctx) {
    if (!ctx) {
        return;
    }
    
    if (ctx->tokens) {
        /* Securely clear token data */
        OPENSSL_cleanse(ctx->tokens, ctx->capacity * sizeof(TokenEntry));
        free(ctx->tokens);
        ctx->tokens = NULL;
    }
    
    free(ctx);
}

/* Get current timestamp in seconds */
static int64_t get_current_timestamp(void) {
    return (int64_t)time(NULL);
}

/* Convert bytes to hexadecimal string */
static int bytes_to_hex(const unsigned char* data, size_t data_len, 
                       char* hex_output, size_t output_size) {
    if (!data || !hex_output || output_size < (data_len * 2 + 1)) {
        return 0;
    }
    
    for (size_t i = 0; i < data_len; i++) {
        int written = snprintf(hex_output + (i * 2), 3, "%02x", data[i]);
        if (written != 2) {
            return 0;
        }
    }
    
    hex_output[data_len * 2] = '\\0';
    return 1;
}

/* Constant-time string comparison to prevent timing attacks */
static int constant_time_compare(const char* a, const char* b, size_t length) {
    if (!a || !b) {
        return 0;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < length; i++) {
        result |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }
    
    return result == 0;
}

/* Validate hex string format */
static int is_valid_hex(const char* str, size_t expected_length) {
    if (!str || strlen(str) != expected_length) {
        return 0;
    }
    
    for (size_t i = 0; i < expected_length; i++) {
        if (!isxdigit((unsigned char)str[i])) {
            return 0;
        }
    }
    
    return 1;
}

/* Remove expired tokens */
static void cleanup_expired_tokens(CSRFContext* ctx) {
    if (!ctx || !ctx->tokens) {
        return;
    }
    
    int64_t current_time = get_current_timestamp();
    
    for (size_t i = 0; i < ctx->capacity; i++) {
        if (ctx->tokens[i].active && ctx->tokens[i].expiration < current_time) {
            /* Securely clear expired token */
            OPENSSL_cleanse(ctx->tokens[i].token, TOKEN_HEX_LENGTH + 1);
            ctx->tokens[i].active = 0;
            if (ctx->count > 0) {
                ctx->count--;
            }
        }
    }
}

/* Generate CSRF token with cryptographically secure randomness */
char* csrf_generate_token(CSRFContext* ctx) {
    if (!ctx || !ctx->tokens) {
        return NULL;
    }
    
    /* Cleanup if token store is full */
    if (ctx->count >= ctx->capacity) {
        cleanup_expired_tokens(ctx);
        if (ctx->count >= ctx->capacity) {
            fprintf(stderr, "Token store is full\\n");
            return NULL;
        }
    }
    
    /* Generate secure random bytes using OpenSSL */
    unsigned char random_bytes[TOKEN_LENGTH];
    if (RAND_bytes(random_bytes, TOKEN_LENGTH) != 1) {
        fprintf(stderr, "Failed to generate secure random bytes\\n");
        return NULL;
    }
    
    /* Convert to hex string */
    char hex_token[TOKEN_HEX_LENGTH + 1];
    if (!bytes_to_hex(random_bytes, TOKEN_LENGTH, hex_token, sizeof(hex_token))) {
        OPENSSL_cleanse(random_bytes, TOKEN_LENGTH);
        return NULL;
    }
    
    /* Securely clear random bytes */
    OPENSSL_cleanse(random_bytes, TOKEN_LENGTH);
    
    /* Find empty slot in token store */
    size_t slot = 0;
    for (slot = 0; slot < ctx->capacity; slot++) {
        if (!ctx->tokens[slot].active) {
            break;
        }
    }
    
    if (slot >= ctx->capacity) {
        return NULL;
    }
    
    /* Store token with expiration */
    strncpy(ctx->tokens[slot].token, hex_token, TOKEN_HEX_LENGTH);
    ctx->tokens[slot].token[TOKEN_HEX_LENGTH] = '\\0';
    ctx->tokens[slot].expiration = get_current_timestamp() + TOKEN_VALIDITY_SECONDS;
    ctx->tokens[slot].active = 1;
    ctx->count++;
    
    /* Return pointer to stored token (valid until cleanup) */
    return ctx->tokens[slot].token;
}

/* Validate CSRF token */
int csrf_validate_token(CSRFContext* ctx, const char* token) {
    if (!ctx || !ctx->tokens || !token) {
        return 0;
    }
    
    /* Validate token format */
    if (!is_valid_hex(token, TOKEN_HEX_LENGTH)) {
        return 0;
    }
    
    int64_t current_time = get_current_timestamp();
    
    /* Search for token in store using constant-time comparison */
    for (size_t i = 0; i < ctx->capacity; i++) {
        if (!ctx->tokens[i].active) {
            continue;
        }
        
        /* Use constant-time comparison for token matching */
        if (constant_time_compare(ctx->tokens[i].token, token, TOKEN_HEX_LENGTH)) {
            /* Check expiration */
            if (ctx->tokens[i].expiration < current_time) {
                /* Token expired */
                OPENSSL_cleanse(ctx->tokens[i].token, TOKEN_HEX_LENGTH + 1);
                ctx->tokens[i].active = 0;
                if (ctx->count > 0) {
                    ctx->count--;
                }
                return 0;
            }
            
            /* Valid token - remove it (single-use pattern) */
            OPENSSL_cleanse(ctx->tokens[i].token, TOKEN_HEX_LENGTH + 1);
            ctx->tokens[i].active = 0;
            if (ctx->count > 0) {
                ctx->count--;
            }
            return 1;
        }
    }
    
    return 0;
}

/* Simulate HTTP request structure */
typedef struct {
    const char* method;
    const char* path;
    const char* csrf_token_header;
    const char* csrf_token_form;
} HTTPRequest;

/* Process request with CSRF protection */
int process_request(CSRFContext* ctx, const HTTPRequest* request) {
    if (!ctx || !request || !request->method) {
        return 0;
    }
    
    /* Allow safe methods without CSRF check */
    if (strcmp(request->method, "GET") == 0 || 
        strcmp(request->method, "HEAD") == 0 ||
        strcmp(request->method, "OPTIONS") == 0) {
        return 1;
    }
    
    /* Extract token from header or form data */
    const char* token = NULL;
    if (request->csrf_token_header) {
        token = request->csrf_token_header;
    } else if (request->csrf_token_form) {
        token = request->csrf_token_form;
    }
    
    /* Require token for state-changing methods */
    if (!token) {
        fprintf(stderr, "CSRF token missing\\n");
        return 0;
    }
    
    /* Validate token */
    if (!csrf_validate_token(ctx, token)) {
        fprintf(stderr, "CSRF token validation failed\\n");
        return 0;
    }
    
    return 1;
}

int main(void) {
    printf("=== CSRF Protection Test Cases ===\\n\\n");
    
    /* Initialize CSRF context */
    CSRFContext* ctx = csrf_init();
    if (!ctx) {
        fprintf(stderr, "Failed to initialize CSRF context\\n");
        return 1;
    }
    
    /* Test Case 1: Generate and validate valid token */
    printf("Test 1: Generate and validate valid token\\n");
    char* token1 = csrf_generate_token(ctx);
    if (token1) {
        printf("Token generated (length: %zu)\\n", strlen(token1));
        /* Make a copy before validation as validation removes it */
        char token_copy[TOKEN_HEX_LENGTH + 1];
        strncpy(token_copy, token1, TOKEN_HEX_LENGTH);
        token_copy[TOKEN_HEX_LENGTH] = '\\0';
        int valid = csrf_validate_token(ctx, token_copy);
        printf("Validation result: %s\\n", valid ? "PASS" : "FAIL");
    } else {
        printf("Token generation failed\\n");
    }
    printf("\\n");
    
    /* Test Case 2: Reject invalid token */
    printf("Test 2: Reject invalid token\\n");
    int invalid_result = csrf_validate_token(ctx, "invalidtoken123");
    printf("Invalid token rejected: %s\\n", !invalid_result ? "PASS" : "FAIL");
    printf("\\n");
    
    /* Test Case 3: Single-use token */
    printf("Test 3: Single-use token enforcement\\n");
    char* token3 = csrf_generate_token(ctx);
    if (token3) {
        char token3_copy[TOKEN_HEX_LENGTH + 1];
        strncpy(token3_copy, token3, TOKEN_HEX_LENGTH);
        token3_copy[TOKEN_HEX_LENGTH] = '\\0';
        
        int first_use = csrf_validate_token(ctx, token3_copy);
        int second_use = csrf_validate_token(ctx, token3_copy);
        printf("First use: %s\\n", first_use ? "valid" : "invalid");
        printf("Second use: %s\\n", second_use ? "valid" : "invalid");
        printf("Single-use enforced: %s\\n", (first_use && !second_use) ? "PASS" : "FAIL");
    }
    printf("\\n");
    
    /* Test Case 4: HTTP POST with valid token */
    printf("Test 4: HTTP POST with valid CSRF token\\n");
    char* token4 = csrf_generate_token(ctx);
    if (token4) {
        char token4_copy[TOKEN_HEX_LENGTH + 1];
        strncpy(token4_copy, token4, TOKEN_HEX_LENGTH);
        token4_copy[TOKEN_HEX_LENGTH] = '\\0';
        
        HTTPRequest post_request = {
            .method = "POST",
            .path = "/api/update",
            .csrf_token_header = token4_copy,
            .csrf_token_form = NULL
        };
        int post_result = process_request(ctx, &post_request);
        printf("POST request processed: %s\\n", post_result ? "PASS" : "FAIL");
    }
    printf("\\n");
    
    /* Test Case 5: HTTP POST without token */
    printf("Test 5: HTTP POST without CSRF token (should fail)\\n");
    HTTPRequest post_no_token = {
        .method = "POST",
        .path = "/api/update",
        .csrf_token_header = NULL,
        .csrf_token_form = NULL
    };
    int post_no_token_result = process_request(ctx, &post_no_token);
    printf("POST without token rejected: %s\\n", !post_no_token_result ? "PASS" : "FAIL");
    
    /* Cleanup */
    csrf_cleanup(ctx);
    
    return 0;
}
