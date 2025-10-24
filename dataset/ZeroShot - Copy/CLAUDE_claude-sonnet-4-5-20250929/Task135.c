
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define TOKEN_LENGTH 32
#define TOKEN_EXPIRY_TIME (30 * 60) // 30 minutes in seconds
#define MAX_SESSIONS 100
#define MAX_TOKEN_STRING_LENGTH 64

typedef struct {
    char sessionId[64];
    char token[MAX_TOKEN_STRING_LENGTH];
    time_t timestamp;
    bool active;
} TokenData;

typedef struct {
    TokenData tokens[MAX_SESSIONS];
    int count;
} CSRFProtection;

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

void base64_encode(const unsigned char* data, size_t len, char* output) {
    int i = 0, j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    int out_idx = 0;
    
    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++) {
                output[out_idx++] = base64_chars[char_array_4[i]];
            }
            i = 0;
        }
    }
    
    if (i) {
        for(j = i; j < 3; j++) {
            char_array_3[j] = '\\0';
        }
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (j = 0; j < i + 1; j++) {
            output[out_idx++] = base64_chars[char_array_4[j]];
        }
    }
    
    output[out_idx] = '\\0';
}

void init_csrf_protection(CSRFProtection* csrf) {
    csrf->count = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        csrf->tokens[i].active = false;
    }
    srand(time(NULL));
}

char* generate_token(CSRFProtection* csrf, const char* sessionId) {
    unsigned char tokenBytes[TOKEN_LENGTH];
    
    for (int i = 0; i < TOKEN_LENGTH; i++) {
        tokenBytes[i] = rand() % 256;
    }
    
    // Find existing session or create new
    int idx = -1;
    for (int i = 0; i < csrf->count; i++) {
        if (strcmp(csrf->tokens[i].sessionId, sessionId) == 0) {
            idx = i;
            break;
        }
    }
    
    if (idx == -1 && csrf->count < MAX_SESSIONS) {
        idx = csrf->count++;
    }
    
    if (idx != -1) {
        strcpy(csrf->tokens[idx].sessionId, sessionId);
        base64_encode(tokenBytes, TOKEN_LENGTH, csrf->tokens[idx].token);
        csrf->tokens[idx].timestamp = time(NULL);
        csrf->tokens[idx].active = true;
        
        return csrf->tokens[idx].token;
    }
    
    return NULL;
}

bool constant_time_equals(const char* a, const char* b) {
    if (strlen(a) != strlen(b)) {
        return false;
    }
    
    int result = 0;
    for (size_t i = 0; i < strlen(a); i++) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

bool validate_token(CSRFProtection* csrf, const char* sessionId, const char* token) {
    if (!sessionId || !token) {
        return false;
    }
    
    for (int i = 0; i < csrf->count; i++) {
        if (csrf->tokens[i].active && strcmp(csrf->tokens[i].sessionId, sessionId) == 0) {
            time_t currentTime = time(NULL);
            
            if (currentTime - csrf->tokens[i].timestamp > TOKEN_EXPIRY_TIME) {
                csrf->tokens[i].active = false;
                return false;
            }
            
            return constant_time_equals(csrf->tokens[i].token, token);
        }
    }
    
    return false;
}

void invalidate_token(CSRFProtection* csrf, const char* sessionId) {
    for (int i = 0; i < csrf->count; i++) {
        if (strcmp(csrf->tokens[i].sessionId, sessionId) == 0) {
            csrf->tokens[i].active = false;
            return;
        }
    }
}

int main() {
    CSRFProtection csrf;
    init_csrf_protection(&csrf);
    
    printf("=== CSRF Protection Test Cases ===\\n\\n");
    
    // Test Case 1: Generate and validate valid token
    printf("Test Case 1: Generate and validate valid token\\n");
    const char* session1 = "session123";
    char* token1 = generate_token(&csrf, session1);
    printf("Generated Token: %s\\n", token1);
    printf("Validation Result: %s\\n", validate_token(&csrf, session1, token1) ? "true" : "false");
    printf("\\n");
    
    // Test Case 2: Validate with wrong token
    printf("Test Case 2: Validate with wrong token\\n");
    const char* session2 = "session456";
    char* token2 = generate_token(&csrf, session2);
    const char* wrongToken = "wrongToken123";
    printf("Generated Token: %s\\n", token2);
    printf("Wrong Token: %s\\n", wrongToken);
    printf("Validation Result: %s\\n", validate_token(&csrf, session2, wrongToken) ? "true" : "false");
    printf("\\n");
    
    // Test Case 3: Validate with non-existent session
    printf("Test Case 3: Validate with non-existent session\\n");
    const char* nonExistentSession = "nonExistent";
    const char* token3 = "someToken";
    printf("Validation Result: %s\\n", validate_token(&csrf, nonExistentSession, token3) ? "true" : "false");
    printf("\\n");
    
    // Test Case 4: Invalidate token and try to use it
    printf("Test Case 4: Invalidate token and try to use it\\n");
    const char* session4 = "session789";
    char* token4 = generate_token(&csrf, session4);
    printf("Generated Token: %s\\n", token4);
    printf("Validation Before Invalidation: %s\\n", validate_token(&csrf, session4, token4) ? "true" : "false");
    invalidate_token(&csrf, session4);
    printf("Validation After Invalidation: %s\\n", validate_token(&csrf, session4, token4) ? "true" : "false");
    printf("\\n");
    
    // Test Case 5: Multiple sessions with different tokens
    printf("Test Case 5: Multiple sessions with different tokens\\n");
    const char* sessionA = "sessionA";
    const char* sessionB = "sessionB";
    char* tokenA = generate_token(&csrf, sessionA);
    char* tokenB = generate_token(&csrf, sessionB);
    printf("Session A Token: %s\\n", tokenA);
    printf("Session B Token: %s\\n", tokenB);
    printf("Validate A with A's token: %s\\n", validate_token(&csrf, sessionA, tokenA) ? "true" : "false");
    printf("Validate B with B's token: %s\\n", validate_token(&csrf, sessionB, tokenB) ? "true" : "false");
    printf("Validate A with B's token: %s\\n", validate_token(&csrf, sessionA, tokenB) ? "true" : "false");
    printf("Validate B with A's token: %s\\n", validate_token(&csrf, sessionB, tokenA) ? "true" : "false");
    
    return 0;
}
