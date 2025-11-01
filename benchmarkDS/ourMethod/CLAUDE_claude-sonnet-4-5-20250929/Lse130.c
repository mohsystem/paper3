
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_PARAM_LEN 1000
#define MAX_TEMPLATE_LEN 100000
#define MAX_RENDERED_LEN 200000

// Response structure
typedef struct {
    char* content;
    int statusCode;
} Response;

// Request structure
typedef struct {
    char* name;
} Request;

// Environment structure
typedef struct {
    Request* request;
    char* template;
} Environment;

// Initialize response with allocated memory
Response* createResponse(const char* content, int statusCode) {
    Response* resp = (Response*)calloc(1, sizeof(Response));
    if (!resp) return NULL;
    
    if (content) {
        size_t len = strlen(content);
        resp->content = (char*)calloc(len + 1, sizeof(char));
        if (!resp->content) {
            free(resp);
            return NULL;
        }
        strncpy(resp->content, content, len);
        resp->content[len] = '\\0';
    } else {
        resp->content = NULL;
    }
    resp->statusCode = statusCode;
    return resp;
}

// Free response memory
void freeResponse(Response* resp) {
    if (resp) {
        if (resp->content) {
            // Clear sensitive data before free
            memset(resp->content, 0, strlen(resp->content));
            free(resp->content);
        }
        free(resp);
    }
}

// HTML escape to prevent XSS - allocates new string
char* escapeHtml(const char* input) {
    if (!input) return NULL;
    
    size_t inputLen = strlen(input);
    // Worst case: every character becomes 6 characters (&quot;)
    size_t maxLen = inputLen * 6 + 1;
    if (maxLen > MAX_RENDERED_LEN) {
        maxLen = MAX_RENDERED_LEN;
    }
    
    char* escaped = (char*)calloc(maxLen, sizeof(char));
    if (!escaped) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < inputLen && j < maxLen - 7; i++) {
        switch (input[i]) {
            case '<':
                if (j + 4 < maxLen) {
                    strcpy(&escaped[j], "&lt;");
                    j += 4;
                }
                break;
            case '>':
                if (j + 4 < maxLen) {
                    strcpy(&escaped[j], "&gt;");
                    j += 4;
                }
                break;
            case '&':
                if (j + 5 < maxLen) {
                    strcpy(&escaped[j], "&amp;");
                    j += 5;
                }
                break;
            case '"':
                if (j + 6 < maxLen) {
                    strcpy(&escaped[j], "&quot;");
                    j += 6;
                }
                break;
            case '\\'':
                if (j + 6 < maxLen) {
                    strcpy(&escaped[j], "&#x27;");
                    j += 6;
                }
                break;
            case '/':
                if (j + 6 < maxLen) {
                    strcpy(&escaped[j], "&#x2F;");
                    j += 6;
                }
                break;
            default:
                if (j < maxLen - 1) {
                    escaped[j++] = input[i];
                }
        }
    }
    escaped[j] = '\\0';
    return escaped;
}

// Simple template rendering with XSS protection
char* renderTemplate(const char* template, const char* name) {
    if (!template) return NULL;
    
    // Escape name to prevent XSS
    char* safeName = escapeHtml(name ? name : "");
    if (!safeName) return NULL;
    
    size_t templateLen = strlen(template);
    size_t nameLen = strlen(safeName);
    
    // Allocate buffer for rendered output
    char* rendered = (char*)calloc(MAX_RENDERED_LEN, sizeof(char));
    if (!rendered) {
        memset(safeName, 0, strlen(safeName));
        free(safeName);
        return NULL;
    }
    
    size_t i = 0, j = 0;
    while (i < templateLen && j < MAX_RENDERED_LEN - 1) {
        // Look for {{ name }}
        if (i + 1 < templateLen && template[i] == '{' && template[i+1] == '{') {
            i += 2;
            // Skip whitespace
            while (i < templateLen && template[i] == ' ') i++;
            
            // Check for "name"
            if (i + 4 <= templateLen && strncmp(&template[i], "name", 4) == 0) {
                i += 4;
                // Skip whitespace
                while (i < templateLen && template[i] == ' ') i++;
                
                // Check for }}
                if (i + 1 < templateLen && template[i] == '}' && template[i+1] == '}') {
                    i += 2;
                    // Insert escaped name
                    size_t copyLen = nameLen;
                    if (j + copyLen >= MAX_RENDERED_LEN) {
                        copyLen = MAX_RENDERED_LEN - j - 1;
                    }
                    strncpy(&rendered[j], safeName, copyLen);
                    j += copyLen;
                    continue;
                }
            }
        }
        rendered[j++] = template[i++];
    }
    rendered[j] = '\\0';
    
    // Clear and free escaped name
    memset(safeName, 0, strlen(safeName));
    free(safeName);
    
    return rendered;
}

// Renders response from environment
// Security: Validates all inputs, escapes output to prevent XSS
Response* renderResponseFromEnv(Environment* env) {
    if (!env) {
        return createResponse("Invalid environment", 400);
    }
    
    if (!env->request) {
        return createResponse("Invalid request", 400);
    }
    
    // Get name parameter - treat as untrusted
    const char* name = env->request->name;
    
    // Get template
    if (!env->template || strlen(env->template) == 0) {
        return createResponse("Template not found", 404);
    }
    
    // Render template with escaped name
    char* rendered = renderTemplate(env->template, name);
    if (!rendered) {
        return createResponse("Internal server error", 500);
    }
    
    Response* resp = createResponse(rendered, 200);
    
    // Clear and free rendered content
    memset(rendered, 0, strlen(rendered));
    free(rendered);
    
    return resp;
}

// Home function that calls renderResponseFromEnv
Response* home(Environment* env) {
    return renderResponseFromEnv(env);
}

// Test cases
int main(void) {
    // Test 1: Normal input
    Request req1 = {.name = "John"};
    Environment env1 = {.request = &req1, .template = "Hello, {{ name }}!"};
    Response* resp1 = home(&env1);
    printf("Test 1: %s (Status: %d)\\n", resp1->content, resp1->statusCode);
    freeResponse(resp1);
    
    // Test 2: XSS attempt - should be escaped
    Request req2 = {.name = "<script>alert('XSS')</script>"};
    Environment env2 = {.request = &req2, .template = "Hello, {{ name }}!"};
    Response* resp2 = home(&env2);
    printf("Test 2: %s (Status: %d)\\n", resp2->content, resp2->statusCode);
    freeResponse(resp2);
    
    // Test 3: Empty name parameter
    Request req3 = {.name = ""};
    Environment env3 = {.request = &req3, .template = "Hello, {{ name }}!"};
    Response* resp3 = home(&env3);
    printf("Test 3: %s (Status: %d)\\n", resp3->content, resp3->statusCode);
    freeResponse(resp3);
    
    // Test 4: NULL name parameter
    Request req4 = {.name = NULL};
    Environment env4 = {.request = &req4, .template = "Hello, {{ name }}!"};
    Response* resp4 = home(&env4);
    printf("Test 4: %s (Status: %d)\\n", resp4->content, resp4->statusCode);
    freeResponse(resp4);
    
    // Test 5: NULL environment
    Response* resp5 = home(NULL);
    printf("Test 5: %s (Status: %d)\\n", resp5->content, resp5->statusCode);
    freeResponse(resp5);
    
    return 0;
}
