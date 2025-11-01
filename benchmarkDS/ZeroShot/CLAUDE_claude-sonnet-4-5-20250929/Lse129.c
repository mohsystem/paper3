
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TEMPLATE_SIZE 1024
#define MAX_NAME_SIZE 256
#define MAX_RESPONSE_SIZE 2048

typedef struct {
    char template_content[MAX_TEMPLATE_SIZE];
} Environment;

// Initialize environment with a simple template
void init_environment(Environment* env) {
    strcpy(env->template_content, "<html><body>Hello, {{name}}!</body></html>");
}

// Sanitize input to prevent XSS attacks
char* sanitize_input(const char* input, char* output, size_t max_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < max_size - 1; i++) {
        if (input[i] == '<') {
            if (j + 4 < max_size - 1) {
                strcpy(&output[j], "&lt;");
                j += 4;
            }
        } else if (input[i] == '>') {
            if (j + 4 < max_size - 1) {
                strcpy(&output[j], "&gt;");
                j += 4;
            }
        } else if (input[i] == '&') {
            if (j + 5 < max_size - 1) {
                strcpy(&output[j], "&amp;");
                j += 5;
            }
        } else if (input[i] == '"') {
            if (j + 6 < max_size - 1) {
                strcpy(&output[j], "&quot;");
                j += 6;
            }
        } else if (input[i] == '\\'') {
            if (j + 6 < max_size - 1) {
                strcpy(&output[j], "&#x27;");
                j += 6;
            }
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
    return output;
}

// Render response from environment
char* render_response_from_env(Environment* env, const char* name, char* response, size_t max_size) {
    char sanitized_name[MAX_NAME_SIZE];
    char* template_str = env->template_content;
    
    // Sanitize the name parameter
    sanitize_input(name, sanitized_name, MAX_NAME_SIZE);
    
    // Find and replace {{name}} in template
    char* placeholder = strstr(template_str, "{{name}}");
    if (placeholder != NULL) {
        size_t prefix_len = placeholder - template_str;
        size_t suffix_start = prefix_len + 8; // length of "{{name}}"
        
        // Build the response
        strncpy(response, template_str, prefix_len);
        response[prefix_len] = '\\0';
        strncat(response, sanitized_name, max_size - strlen(response) - 1);
        strncat(response, &template_str[suffix_start], max_size - strlen(response) - 1);
    } else {
        strncpy(response, template_str, max_size - 1);
        response[max_size - 1] = '\\0';
    }
    
    return response;
}

// Home function
char* home(Environment* env, const char* name, char* response, size_t max_size) {
    return render_response_from_env(env, name, response, max_size);
}

int main() {
    Environment env;
    init_environment(&env);
    
    // Test cases
    const char* test_cases[] = {
        "John",
        "Alice",
        "<script>alert('XSS')</script>",
        "Bob O'Brien",
        ""
    };
    
    printf("Testing render_response_from_env and home functions:\\n");
    printf("===================================================\\n\\n");
    
    for (int i = 0; i < 5; i++) {
        char response[MAX_RESPONSE_SIZE];
        printf("Test Case %d:\\n", i + 1);
        printf("Input: %s\\n", test_cases[i]);
        home(&env, test_cases[i], response, MAX_RESPONSE_SIZE);
        printf("Response: %s\\n\\n", response);
    }
    
    return 0;
}
