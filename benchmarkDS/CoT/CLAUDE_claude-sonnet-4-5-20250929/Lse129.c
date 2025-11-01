
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>

#define MAX_STRING_LEN 256
#define MAX_TEMPLATES 10

typedef struct {
    char name[MAX_STRING_LEN];
    char content[MAX_STRING_LEN];
} Template;

typedef struct {
    Template templates[MAX_TEMPLATES];
    int count;
} Environment;

typedef struct {
    char name[MAX_STRING_LEN];
    char value[MAX_STRING_LEN];
} Parameter;

typedef struct {
    Parameter parameters[10];
    int count;
} Request;

typedef struct {
    char content[MAX_STRING_LEN * 2];
} Response;

void initEnvironment(Environment* env) {
    env->count = 2;
    strcpy(env->templates[0].name, "greeting");
    strcpy(env->templates[0].content, "Hello, {{name}}! Welcome to our application.");
    strcpy(env->templates[1].name, "default");
    strcpy(env->templates[1].content, "Welcome, {{name}}!");
}

char* getTemplate(Environment* env, const char* templateName) {
    for (int i = 0; i < env->count; i++) {
        if (strcmp(env->templates[i].name, templateName) == 0) {
            return env->templates[i].content;
        }
    }
    return env->templates[1].content;
}

void setParameter(Request* req, const char* key, const char* value) {
    if (req->count < 10) {
        strcpy(req->parameters[req->count].name, key);
        strcpy(req->parameters[req->count].value, value);
        req->count++;
    }
}

char* getParameter(Request* req, const char* key) {
    for (int i = 0; i < req->count; i++) {
        if (strcmp(req->parameters[i].name, key) == 0) {
            return req->parameters[i].value;
        }
    }
    return NULL;
}

int isSafeName(const char* input) {
    if (input == NULL || strlen(input) == 0 || strlen(input) > 50) {
        return 0;
    }
    
    for (int i = 0; input[i] != '\\0'; i++) {
        if (!isalnum(input[i]) && input[i] != '_' && input[i] != ' ' && input[i] != '-') {
            return 0;
        }
    }
    return 1;
}

void sanitizeInput(const char* input, char* output) {
    if (input == NULL || strlen(input) == 0 || !isSafeName(input)) {
        strcpy(output, "Guest");
        return;
    }
    
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < MAX_STRING_LEN - 1; i++) {
        if (input[i] != '<' && input[i] != '>' && input[i] != '"' && 
            input[i] != '\\'' && input[i] != '&') {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

void escapeHtml(const char* input, char* output) {
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < MAX_STRING_LEN * 2 - 10; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&output[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&output[j], "&#x27;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

Response renderResponseFromEnv(Environment* env, Request* req) {
    Response response;
    char* nameParam = getParameter(req, "name");
    char sanitized[MAX_STRING_LEN];
    char escaped[MAX_STRING_LEN * 2];
    
    sanitizeInput(nameParam, sanitized);
    escapeHtml(sanitized, escaped);
    
    char* templateStr = getTemplate(env, "greeting");
    char* placeholder = strstr(templateStr, "{{name}}");
    
    if (placeholder != NULL) {
        int prefixLen = placeholder - templateStr;
        strncpy(response.content, templateStr, prefixLen);
        response.content[prefixLen] = '\\0';
        strcat(response.content, escaped);
        strcat(response.content, placeholder + 8);
    } else {
        strcpy(response.content, templateStr);
    }
    
    return response;
}

Response home(Environment* env, Request* req) {
    return renderResponseFromEnv(env, req);
}

int main() {
    Environment env;
    initEnvironment(&env);
    
    // Test case 1: Valid name
    Request req1 = {.count = 0};
    setParameter(&req1, "name", "John");
    Response resp1 = home(&env, &req1);
    printf("Test 1: %s\\n", resp1.content);
    
    // Test case 2: Name with spaces
    Request req2 = {.count = 0};
    setParameter(&req2, "name", "Jane Doe");
    Response resp2 = home(&env, &req2);
    printf("Test 2: %s\\n", resp2.content);
    
    // Test case 3: Null/empty name
    Request req3 = {.count = 0};
    setParameter(&req3, "name", "");
    Response resp3 = home(&env, &req3);
    printf("Test 3: %s\\n", resp3.content);
    
    // Test case 4: Name with special characters
    Request req4 = {.count = 0};
    setParameter(&req4, "name", "<script>alert('xss')</script>");
    Response resp4 = home(&env, &req4);
    printf("Test 4: %s\\n", resp4.content);
    
    // Test case 5: Name with invalid characters
    Request req5 = {.count = 0};
    setParameter(&req5, "name", "User@123!");
    Response resp5 = home(&env, &req5);
    printf("Test 5: %s\\n", resp5.content);
    
    return 0;
}
