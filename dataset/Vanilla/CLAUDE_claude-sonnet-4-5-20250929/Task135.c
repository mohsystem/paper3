
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define TOKEN_LENGTH 32
#define MAX_TOKENS 100
#define TOKEN_EXPIRY_MS 1800000 // 30 minutes in milliseconds

typedef struct {
    char token[128];
    long long created_at;
    char session_id[64];
    bool active;
} TokenData;

typedef struct {
    TokenData tokens[MAX_TOKENS];
    int count;
} Task135;

static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

void base64_encode(const unsigned char* data, size_t len, char* output) {
    int i = 0, j = 0;
    unsigned char array_3[3];
    unsigned char array_4[4];
    size_t pos = 0;

    while (len--) {
        array_3[i++] = *(data++);
        if (i == 3) {
            array_4[0] = (array_3[0] & 0xfc) >> 2;
            array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
            array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);
            array_4[3] = array_3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                output[pos++] = base64_chars[array_4[i]];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++)
            array_3[j] = '\\0';

        array_4[0] = (array_3[0] & 0xfc) >> 2;
        array_4[1] = ((array_3[0] & 0x03) << 4) + ((array_3[1] & 0xf0) >> 4);
        array_4[2] = ((array_3[1] & 0x0f) << 2) + ((array_3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            output[pos++] = base64_chars[array_4[j]];
    }
    output[pos] = '\\0';
}

long long get_current_time_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}

void init_csrf(Task135* csrf) {
    csrf->count = 0;
    for (int i = 0; i < MAX_TOKENS; i++) {
        csrf->tokens[i].active = false;
    }
    srand(time(NULL));
}

void generate_token(Task135* csrf, const char* session_id, char* output) {
    unsigned char random_bytes[TOKEN_LENGTH];
    for (int i = 0; i < TOKEN_LENGTH; i++) {
        random_bytes[i] = (unsigned char)(rand() % 256);
    }
    
    char token[128];
    base64_encode(random_bytes, TOKEN_LENGTH, token);
    
    int index = -1;
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (strcmp(csrf->tokens[i].session_id, session_id) == 0 && csrf->tokens[i].active) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        for (int i = 0; i < MAX_TOKENS; i++) {
            if (!csrf->tokens[i].active) {
                index = i;
                csrf->count++;
                break;
            }
        }
    }
    
    if (index != -1) {
        strcpy(csrf->tokens[index].token, token);
        strcpy(csrf->tokens[index].session_id, session_id);
        csrf->tokens[index].created_at = get_current_time_ms();
        csrf->tokens[index].active = true;
    }
    
    strcpy(output, token);
}

bool validate_token(Task135* csrf, const char* session_id, const char* token) {
    if (session_id == NULL || token == NULL || strlen(session_id) == 0 || strlen(token) == 0) {
        return false;
    }
    
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (csrf->tokens[i].active && strcmp(csrf->tokens[i].session_id, session_id) == 0) {
            long long now = get_current_time_ms();
            if (now - csrf->tokens[i].created_at > TOKEN_EXPIRY_MS) {
                csrf->tokens[i].active = false;
                csrf->count--;
                return false;
            }
            return strcmp(csrf->tokens[i].token, token) == 0;
        }
    }
    return false;
}

void invalidate_token(Task135* csrf, const char* session_id) {
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (csrf->tokens[i].active && strcmp(csrf->tokens[i].session_id, session_id) == 0) {
            csrf->tokens[i].active = false;
            csrf->count--;
            break;
        }
    }
}

int get_active_token_count(Task135* csrf) {
    long long now = get_current_time_ms();
    int count = 0;
    for (int i = 0; i < MAX_TOKENS; i++) {
        if (csrf->tokens[i].active) {
            if (now - csrf->tokens[i].created_at > TOKEN_EXPIRY_MS) {
                csrf->tokens[i].active = false;
            } else {
                count++;
            }
        }
    }
    csrf->count = count;
    return count;
}

int main() {
    Task135 csrf;
    init_csrf(&csrf);
    
    // Test Case 1: Generate and validate token
    printf("Test Case 1: Generate and validate token\\n");
    char token1[128];
    generate_token(&csrf, "session_123", token1);
    printf("Generated token: %s\\n", token1);
    printf("Token valid: %s\\n", validate_token(&csrf, "session_123", token1) ? "true" : "false");
    printf("\\n");
    
    // Test Case 2: Invalid token validation
    printf("Test Case 2: Invalid token validation\\n");
    char token2[128];
    generate_token(&csrf, "session_456", token2);
    printf("Valid token check: %s\\n", validate_token(&csrf, "session_456", token2) ? "true" : "false");
    printf("Invalid token check: %s\\n", validate_token(&csrf, "session_456", "invalid_token") ? "true" : "false");
    printf("\\n");
    
    // Test Case 3: Token invalidation
    printf("Test Case 3: Token invalidation\\n");
    char token3[128];
    generate_token(&csrf, "session_789", token3);
    printf("Before invalidation: %s\\n", validate_token(&csrf, "session_789", token3) ? "true" : "false");
    invalidate_token(&csrf, "session_789");
    printf("After invalidation: %s\\n", validate_token(&csrf, "session_789", token3) ? "true" : "false");
    printf("\\n");
    
    // Test Case 4: Multiple sessions
    printf("Test Case 4: Multiple sessions\\n");
    char tokenA[128], tokenB[128];
    generate_token(&csrf, "session_A", tokenA);
    generate_token(&csrf, "session_B", tokenB);
    printf("Session A valid: %s\\n", validate_token(&csrf, "session_A", tokenA) ? "true" : "false");
    printf("Session B valid: %s\\n", validate_token(&csrf, "session_B", tokenB) ? "true" : "false");
    printf("Cross-session invalid: %s\\n", validate_token(&csrf, "session_A", tokenB) ? "true" : "false");
    printf("Active tokens: %d\\n", get_active_token_count(&csrf));
    printf("\\n");
    
    // Test Case 5: NULL handling
    printf("Test Case 5: NULL handling\\n");
    printf("NULL session: %s\\n", validate_token(&csrf, NULL, "token") ? "true" : "false");
    printf("NULL token: %s\\n", validate_token(&csrf, "session", NULL) ? "true" : "false");
    printf("Both NULL: %s\\n", validate_token(&csrf, NULL, NULL) ? "true" : "false");
    
    return 0;
}
