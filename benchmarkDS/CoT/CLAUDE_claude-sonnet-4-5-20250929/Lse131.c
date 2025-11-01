
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 51
#define MAX_TEMPLATE_LENGTH 256
#define MAX_RESPONSE_LENGTH 512

typedef struct {
    char greeting_template[MAX_TEMPLATE_LENGTH];
} Environment;

// Initialize environment with template
void init_environment(Environment* env) {
    strcpy(env->greeting_template, "Hello, {{name}}!");
}

// Sanitize input to prevent injection attacks
void sanitize_input(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < output_size - 1; i++) {
        char c = input[i];
        // Only allow alphanumeric and spaces
        if (isalnum(c) || c == ' ') {
            output[j++] = c;
        }
    }
    output[j] = '\\0';
    
    // Trim trailing spaces
    while (j > 0 && output[j-1] == ' ') {
        output[--j] = '\\0';
    }
}

// Validate name parameter
bool is_valid_name(const char* name) {
    if (name == NULL || strlen(name) == 0 || strlen(name) > 50) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(name); i++) {
        if (!isalnum(name[i]) && name[i] != ' ') {
            return false;
        }
    }
    return true;
}

// Render response from environment
void render_response_from_env(Environment* env, const char* name_param, 
                              char* response, size_t response_size) {
    // Validate input
    if (!is_valid_name(name_param)) {
        strncpy(response, "Invalid name parameter", response_size - 1);
        response[response_size - 1] = '\\0';
        return;
    }
    
    // Sanitize input
    char sanitized_name[MAX_NAME_LENGTH];
    sanitize_input(name_param, sanitized_name, MAX_NAME_LENGTH);
    
    // Find and replace {{name}} with sanitized input
    char* placeholder = strstr(env->greeting_template, "{{name}}");
    if (placeholder != NULL) {
        size_t prefix_len = placeholder - env->greeting_template;
        strncpy(response, env->greeting_template, prefix_len);
        response[prefix_len] = '\\0';
        strncat(response, sanitized_name, response_size - strlen(response) - 1);
        strncat(response, placeholder + 8, response_size - strlen(response) - 1);
    } else {
        strncpy(response, env->greeting_template, response_size - 1);
        response[response_size - 1] = '\\0';
    }
}

// Home function
void home(Environment* env, const char* name_param, 
         char* response, size_t response_size) {
    render_response_from_env(env, name_param, response, response_size);
}

int main() {
    Environment env;
    init_environment(&env);
    
    char response[MAX_RESPONSE_LENGTH];
    
    // Test cases
    printf("Test 1: ");
    home(&env, "John", response, MAX_RESPONSE_LENGTH);
    printf("%s\\n", response);
    
    printf("Test 2: ");
    home(&env, "Alice Smith", response, MAX_RESPONSE_LENGTH);
    printf("%s\\n", response);
    
    printf("Test 3: ");
    home(&env, "<script>alert('xss')</script>", response, MAX_RESPONSE_LENGTH);
    printf("%s\\n", response);
    
    printf("Test 4: ");
    home(&env, "{{malicious}}", response, MAX_RESPONSE_LENGTH);
    printf("%s\\n", response);
    
    printf("Test 5: ");
    home(&env, "User123", response, MAX_RESPONSE_LENGTH);
    printf("%s\\n", response);
    
    return 0;
}
