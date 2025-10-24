
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_SESSIONS 100
#define MAX_TOKEN_LENGTH 128
#define MAX_INPUT_LENGTH 256
#define TOKEN_VALIDITY 3600

typedef struct {
    char session_id[MAX_TOKEN_LENGTH];
    char token[MAX_TOKEN_LENGTH];
    time_t timestamp;
} SessionToken;

typedef struct {
    char username[MAX_INPUT_LENGTH];
    char email[MAX_INPUT_LENGTH];
    char theme[MAX_INPUT_LENGTH];
} UserSettings;

typedef struct {
    SessionToken tokens[MAX_SESSIONS];
    int token_count;
    UserSettings users[MAX_SESSIONS];
    int user_count;
} Task40;

void initialize_task40(Task40* app) {
    app->token_count = 0;
    app->user_count = 0;
}

void generate_secure_token(char* token, size_t length) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    srand((unsigned int)time(NULL) + rand());
    
    for (size_t i = 0; i < length - 1; i++) {
        int key = rand() % (sizeof(charset) - 1);
        token[i] = charset[key];
    }
    token[length - 1] = '\\0';
}

int constant_time_compare(const char* a, const char* b) {
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    
    if (len_a != len_b) {
        return 0;
    }
    
    volatile int result = 0;
    for (size_t i = 0; i < len_a; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

void sanitize_input(const char* input, char* output, size_t max_length) {
    if (input == NULL || output == NULL) {
        if (output) output[0] = '\\0';
        return;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < max_length - 1; i++) {
        char c = input[i];
        if (c != '<' && c != '>' && c != '"' && c != '\\'' && c != '&') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
    
    // Trim whitespace
    while (j > 0 && isspace((unsigned char)output[j-1])) {
        output[--j] = '\\0';
    }
}

int validate_email(const char* email) {
    if (email == NULL || strlen(email) == 0) {
        return 0;
    }
    
    const char* at = strchr(email, '@');
    if (at == NULL || at == email) {
        return 0;
    }
    
    const char* dot = strrchr(at, '.');
    if (dot == NULL || dot == at + 1 || *(dot + 1) == '\\0') {
        return 0;
    }
    
    return 1;
}

char* generate_csrf_token(Task40* app, const char* session_id) {
    if (session_id == NULL || strlen(session_id) == 0) {
        return NULL;
    }
    
    if (app->token_count >= MAX_SESSIONS) {
        return NULL;
    }
    
    SessionToken* st = &app->tokens[app->token_count];
    strncpy(st->session_id, session_id, MAX_TOKEN_LENGTH - 1);
    st->session_id[MAX_TOKEN_LENGTH - 1] = '\\0';
    
    generate_secure_token(st->token, MAX_TOKEN_LENGTH);
    st->timestamp = time(NULL);
    
    app->token_count++;
    
    static char token_copy[MAX_TOKEN_LENGTH];
    strncpy(token_copy, st->token, MAX_TOKEN_LENGTH - 1);
    token_copy[MAX_TOKEN_LENGTH - 1] = '\\0';
    
    return token_copy;
}

int validate_csrf_token(Task40* app, const char* session_id, const char* token) {
    if (session_id == NULL || token == NULL) {
        return 0;
    }
    
    for (int i = 0; i < app->token_count; i++) {
        if (strcmp(app->tokens[i].session_id, session_id) == 0) {
            time_t current_time = time(NULL);
            if (current_time - app->tokens[i].timestamp > TOKEN_VALIDITY) {
                return 0;
            }
            
            return constant_time_compare(app->tokens[i].token, token);
        }
    }
    
    return 0;
}

void remove_token(Task40* app, const char* session_id) {
    for (int i = 0; i < app->token_count; i++) {
        if (strcmp(app->tokens[i].session_id, session_id) == 0) {
            for (int j = i; j < app->token_count - 1; j++) {
                app->tokens[j] = app->tokens[j + 1];
            }
            app->token_count--;
            break;
        }
    }
}

char* update_user_settings(Task40* app, const char* session_id, const char* csrf_token,
                          const char* username, const char* email, const char* theme) {
    static char result[512];
    
    if (username == NULL || strlen(username) == 0 || strspn(username, " \\t\\n\\r") == strlen(username)) {
        strcpy(result, "Error: Username cannot be empty");
        return result;
    }
    
    if (!validate_email(email)) {
        strcpy(result, "Error: Invalid email format");
        return result;
    }
    
    if (theme == NULL || strlen(theme) == 0 || strspn(theme, " \\t\\n\\r") == strlen(theme)) {
        strcpy(result, "Error: Theme cannot be empty");
        return result;
    }
    
    if (!validate_csrf_token(app, session_id, csrf_token)) {
        strcpy(result, "Error: Invalid CSRF token. Request rejected for security reasons.");
        return result;
    }
    
    char clean_username[MAX_INPUT_LENGTH];
    char clean_email[MAX_INPUT_LENGTH];
    char clean_theme[MAX_INPUT_LENGTH];
    
    sanitize_input(username, clean_username, MAX_INPUT_LENGTH);
    sanitize_input(email, clean_email, MAX_INPUT_LENGTH);
    sanitize_input(theme, clean_theme, MAX_INPUT_LENGTH);
    
    if (app->user_count < MAX_SESSIONS) {
        UserSettings* settings = &app->users[app->user_count++];
        strncpy(settings->username, clean_username, MAX_INPUT_LENGTH - 1);
        strncpy(settings->email, clean_email, MAX_INPUT_LENGTH - 1);
        strncpy(settings->theme, clean_theme, MAX_INPUT_LENGTH - 1);
    }
    
    remove_token(app, session_id);
    char* new_token = generate_csrf_token(app, session_id);
    
    snprintf(result, sizeof(result), "Success: Settings updated. New CSRF token: %s", 
             new_token ? new_token : "ERROR");
    
    return result;
}

int main() {
    printf("=== CSRF-Protected User Settings Update Demo ===\\n\\n");
    
    Task40 app;
    initialize_task40(&app);
    
    // Test Case 1: Valid update
    printf("Test Case 1: Valid update\\n");
    char* token1 = generate_csrf_token(&app, "session_12345");
    printf("Generated CSRF Token: %s\\n", token1);
    char* result1 = update_user_settings(&app, "session_12345", token1, "john_doe", "john@example.com", "dark");
    printf("%s\\n\\n", result1);
    
    // Test Case 2: Invalid token
    printf("Test Case 2: Invalid CSRF token\\n");
    generate_csrf_token(&app, "session_67890");
    char* result2 = update_user_settings(&app, "session_67890", "invalid_token", "jane_doe", "jane@example.com", "light");
    printf("%s\\n\\n", result2);
    
    // Test Case 3: Token reuse
    printf("Test Case 3: Token reuse attempt\\n");
    char* token3 = generate_csrf_token(&app, "session_11111");
    update_user_settings(&app, "session_11111", token3, "alice", "alice@example.com", "blue");
    char* result3 = update_user_settings(&app, "session_11111", token3, "alice_updated", "alice2@example.com", "red");
    printf("%s\\n\\n", result3);
    
    // Test Case 4: Invalid email
    printf("Test Case 4: Invalid email format\\n");
    char* token4 = generate_csrf_token(&app, "session_22222");
    char* result4 = update_user_settings(&app, "session_22222", token4, "bob", "invalid-email", "green");
    printf("%s\\n\\n", result4);
    
    // Test Case 5: Empty username
    printf("Test Case 5: Empty username\\n");
    char* token5 = generate_csrf_token(&app, "session_33333");
    char* result5 = update_user_settings(&app, "session_33333", token5, "", "test@example.com", "yellow");
    printf("%s\\n\\n", result5);
    
    return 0;
}
