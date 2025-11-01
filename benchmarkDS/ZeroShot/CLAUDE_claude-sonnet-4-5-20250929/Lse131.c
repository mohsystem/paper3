
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_TEMPLATE_SIZE 1024
#define MAX_NAME_SIZE 256
#define MAX_OUTPUT_SIZE 2048

// Environment structure
typedef struct {
    char template[MAX_TEMPLATE_SIZE];
} Environment;

// Request structure
typedef struct {
    char name[MAX_NAME_SIZE];
} Request;

// Response structure
typedef struct {
    char content[MAX_OUTPUT_SIZE];
} Response;

// Initialize environment with template
void init_environment(Environment* env) {
    strncpy(env->template, "Hello, {{name}}! Welcome to our secure application.", 
            MAX_TEMPLATE_SIZE - 1);
    env->template[MAX_TEMPLATE_SIZE - 1] = '\\0';
}

// Create request with name parameter
void init_request(Request* req, const char* name) {
    strncpy(req->name, name, MAX_NAME_SIZE - 1);
    req->name[MAX_NAME_SIZE - 1] = '\\0';
}

// HTML encode function to prevent XSS
void html_encode(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < output_size - 10; i++) {
        switch (input[i]) {
            case '&':
                if (j < output_size - 5) {
                    strcpy(&output[j], "&amp;");
                    j += 5;
                }
                break;
            case '<':
                if (j < output_size - 4) {
                    strcpy(&output[j], "&lt;");
                    j += 4;
                }
                break;
            case '>':
                if (j < output_size - 4) {
                    strcpy(&output[j], "&gt;");
                    j += 4;
                }
                break;
            case '"':
                if (j < output_size - 6) {
                    strcpy(&output[j], "&quot;");
                    j += 6;
                }
                break;
            case '\\'':
                if (j < output_size - 6) {
                    strcpy(&output[j], "&#x27;");
                    j += 6;
                }
                break;
            case '/':
                if (j < output_size - 6) {
                    strcpy(&output[j], "&#x2F;");
                    j += 6;
                }
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\\0';
}

// Check if string is empty or whitespace only
int is_empty_or_whitespace(const char* str) {
    if (str == NULL || str[0] == '\\0') return 1;
    for (size_t i = 0; str[i] != '\\0'; i++) {
        if (!isspace((unsigned char)str[i])) return 0;
    }
    return 1;
}

// Render template with parameter substitution
void render_template(const char* template, const char* name, char* output, size_t output_size) {
    char encoded_name[MAX_OUTPUT_SIZE];
    html_encode(name, encoded_name, sizeof(encoded_name));
    
    const char* placeholder = "{{name}}";
    const char* pos = strstr(template, placeholder);
    
    if (pos != NULL) {
        size_t prefix_len = pos - template;
        size_t suffix_start = prefix_len + strlen(placeholder);
        
        // Copy prefix
        strncpy(output, template, prefix_len);
        output[prefix_len] = '\\0';
        
        // Append encoded name
        strncat(output, encoded_name, output_size - strlen(output) - 1);
        
        // Append suffix
        strncat(output, template + suffix_start, output_size - strlen(output) - 1);
    } else {
        strncpy(output, template, output_size - 1);
        output[output_size - 1] = '\\0';
    }
}

// Render response from environment
Response render_response_from_env(Environment* env, Request* req) {
    Response response;
    char validated_name[MAX_NAME_SIZE];
    
    // Input validation
    if (is_empty_or_whitespace(req->name)) {
        strcpy(validated_name, "Guest");
    } else {
        strncpy(validated_name, req->name, MAX_NAME_SIZE - 1);
        validated_name[MAX_NAME_SIZE - 1] = '\\0';
        
        // Length validation
        if (strlen(validated_name) > 100) {
            validated_name[100] = '\\0';
        }
    }
    
    // Render template
    render_template(env->template, validated_name, response.content, MAX_OUTPUT_SIZE);
    
    return response;
}

// Home function
Response home(Environment* env, Request* req) {
    return render_response_from_env(env, req);
}

int main() {
    Environment env;
    init_environment(&env);
    
    // Test case 1: Normal input
    Request req1;
    init_request(&req1, "Alice");
    Response resp1 = home(&env, &req1);
    printf("Test 1: %s\\n", resp1.content);
    
    // Test case 2: Input with special characters (XSS attempt)
    Request req2;
    init_request(&req2, "<script>alert('XSS')</script>");
    Response resp2 = home(&env, &req2);
    printf("Test 2: %s\\n", resp2.content);
    
    // Test case 3: Empty input
    Request req3;
    init_request(&req3, "");
    Response resp3 = home(&env, &req3);
    printf("Test 3: %s\\n", resp3.content);
    
    // Test case 4: Input with HTML tags
    Request req4;
    init_request(&req4, "<b>Bob</b>");
    Response resp4 = home(&env, &req4);
    printf("Test 4: %s\\n", resp4.content);
    
    // Test case 5: Input with quotes
    Request req5;
    init_request(&req5, "O'Reilly & Sons");
    Response resp5 = home(&env, &req5);
    printf("Test 5: %s\\n", resp5.content);
    
    return 0;
}
