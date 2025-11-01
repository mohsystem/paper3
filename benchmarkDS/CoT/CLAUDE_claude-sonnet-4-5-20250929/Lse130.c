
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_NAME_LENGTH 50
#define MAX_TEMPLATE_LENGTH 256
#define MAX_RESPONSE_LENGTH 512

typedef struct {
    char name[MAX_NAME_LENGTH + 1];
} Request;

typedef struct {
    char template[MAX_TEMPLATE_LENGTH + 1];
    Request request;
} Environment;

typedef struct {
    char content[MAX_RESPONSE_LENGTH + 1];
} Response;

// Security: Validate if character is safe
int is_safe_char(char c) {
    return isalnum(c) || c == '_' || c == ' ' || c == '-';
}

// Security: Sanitize input to prevent injection attacks
void sanitize_input(const char* input, char* output, size_t max_len) {
    if (input == NULL || output == NULL) {
        output[0] = '\\0';
        return;
    }
    
    size_t j = 0;
    size_t input_len = strlen(input);
    
    // Validate length
    if (input_len > MAX_NAME_LENGTH) {
        output[0] = '\\0';
        return;
    }
    
    // Validate and escape characters
    for (size_t i = 0; i < input_len && j < max_len - 6; i++) {
        if (is_safe_char(input[i])) {
            output[j++] = input[i];
        } else if (input[i] == '&') {
            strcpy(&output[j], "&amp;");
            j += 5;
        } else if (input[i] == '<') {
            strcpy(&output[j], "&lt;");
            j += 4;
        } else if (input[i] == '>') {
            strcpy(&output[j], "&gt;");
            j += 4;
        } else if (input[i] == '"') {
            strcpy(&output[j], "&quot;");
            j += 6;
        } else if (input[i] == '\\'') {
            strcpy(&output[j], "&#x27;");
            j += 6;
        } else {
            // Skip invalid characters
            continue;
        }
    }
    
    output[j] = '\\0';
}

Response render_response_from_env(Environment* env) {
    Response response;
    char safe_name[MAX_RESPONSE_LENGTH];
    
    if (env == NULL) {
        strcpy(response.content, "Error: Invalid environment");
        return response;
    }
    
    // Security: Sanitize name parameter
    sanitize_input(env->request.name, safe_name, sizeof(safe_name));
    
    if (strlen(safe_name) == 0) {
        strcpy(safe_name, "Guest");
    }
    
    // Security: Safe template rendering
    char* placeholder = strstr(env->template, "{name}");
    if (placeholder != NULL) {
        size_t prefix_len = placeholder - env->template;
        strncpy(response.content, env->template, prefix_len);
        response.content[prefix_len] = '\\0';
        strcat(response.content, safe_name);
        strcat(response.content, placeholder + 6);
    } else {
        strcpy(response.content, env->template);
    }
    
    return response;
}

Response home(Environment* env) {
    return render_response_from_env(env);
}

int main() {
    // Test Case 1: Normal valid name
    Environment env1;
    strcpy(env1.template, "Hello, {name}!");
    strcpy(env1.request.name, "John");
    Response resp1 = home(&env1);
    printf("Test 1: %s\\n", resp1.content);
    
    // Test Case 2: Name with XSS attempt
    Environment env2;
    strcpy(env2.template, "Welcome, {name}!");
    strcpy(env2.request.name, "<script>alert('XSS')</script>");
    Response resp2 = home(&env2);
    printf("Test 2: %s\\n", resp2.content);
    
    // Test Case 3: Empty name parameter
    Environment env3;
    strcpy(env3.template, "Hi, {name}!");
    env3.request.name[0] = '\\0';
    Response resp3 = home(&env3);
    printf("Test 3: %s\\n", resp3.content);
    
    // Test Case 4: Name with apostrophe
    Environment env4;
    strcpy(env4.template, "Greetings, {name}!");
    strcpy(env4.request.name, "O'Brien");
    Response resp4 = home(&env4);
    printf("Test 4: %s\\n", resp4.content);
    
    // Test Case 5: Long name (should be rejected)
    Environment env5;
    strcpy(env5.template, "Hello, {name}!");
    memset(env5.request.name, 'A', MAX_NAME_LENGTH);
    env5.request.name[MAX_NAME_LENGTH] = '\\0';
    Response resp5 = home(&env5);
    printf("Test 5: %s\\n", resp5.content);
    
    return 0;
}
