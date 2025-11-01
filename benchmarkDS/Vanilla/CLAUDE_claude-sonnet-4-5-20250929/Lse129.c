
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TEMPLATE_SIZE 256
#define MAX_PARAM_SIZE 100

typedef struct {
    char template[MAX_TEMPLATE_SIZE];
} Environment;

typedef struct {
    char name[MAX_PARAM_SIZE];
} Request;

typedef struct {
    char content[MAX_TEMPLATE_SIZE];
} Response;

void initEnvironment(Environment* env) {
    strcpy(env->template, "Hello, {{name}}! Welcome to our service.");
}

char* getTemplate(Environment* env) {
    return env->template;
}

void render(char* result, const char* template, const char* name) {
    char* placeholder = "{{name}}";
    char* pos = strstr(template, placeholder);
    
    if (pos != NULL) {
        size_t beforeLen = pos - template;
        strncpy(result, template, beforeLen);
        result[beforeLen] = '\\0';
        strcat(result, name);
        strcat(result, pos + strlen(placeholder));
    } else {
        strcpy(result, template);
    }
}

void parseRequest(Request* request, const char* queryString) {
    if (queryString != NULL && strlen(queryString) > 0) {
        const char* nameParam = strstr(queryString, "name=");
        if (nameParam != NULL) {
            nameParam += 5; // Skip "name="
            const char* end = strchr(nameParam, '&');
            size_t len = end ? (size_t)(end - nameParam) : strlen(nameParam);
            strncpy(request->name, nameParam, len);
            request->name[len] = '\\0';
        } else {
            strcpy(request->name, "Guest");
        }
    } else {
        strcpy(request->name, "Guest");
    }
}

Response renderResponseFromEnv(Environment* env, Request* request) {
    Response response;
    char* template = getTemplate(env);
    render(response.content, template, request->name);
    return response;
}

Response home(Environment* env, Request* request) {
    return renderResponseFromEnv(env, request);
}

int main() {
    Environment env;
    initEnvironment(&env);
    
    // Test case 1: Name parameter provided
    Request req1;
    parseRequest(&req1, "name=John");
    Response resp1 = home(&env, &req1);
    printf("Test 1: %s\\n", resp1.content);
    
    // Test case 2: Name parameter with different value
    Request req2;
    parseRequest(&req2, "name=Alice");
    Response resp2 = home(&env, &req2);
    printf("Test 2: %s\\n", resp2.content);
    
    // Test case 3: No name parameter (should default to Guest)
    Request req3;
    parseRequest(&req3, "");
    Response resp3 = home(&env, &req3);
    printf("Test 3: %s\\n", resp3.content);
    
    // Test case 4: Name parameter with another value
    Request req4;
    parseRequest(&req4, "name=Bob");
    Response resp4 = home(&env, &req4);
    printf("Test 4: %s\\n", resp4.content);
    
    // Test case 5: Name parameter with special name
    Request req5;
    parseRequest(&req5, "name=Charlie");
    Response resp5 = home(&env, &req5);
    printf("Test 5: %s\\n", resp5.content);
    
    return 0;
}
