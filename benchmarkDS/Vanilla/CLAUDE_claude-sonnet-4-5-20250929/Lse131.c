
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LEN 256
#define MAX_TEMPLATES 10

typedef struct {
    char content[MAX_STRING_LEN];
} Template;

typedef struct {
    Template templates[MAX_TEMPLATES];
    char templateNames[MAX_TEMPLATES][MAX_STRING_LEN];
    int templateCount;
} Environment;

typedef struct {
    char name[MAX_STRING_LEN];
} Request;

typedef struct {
    char body[MAX_STRING_LEN];
} Response;

void initEnvironment(Environment* env) {
    env->templateCount = 1;
    strcpy(env->templateNames[0], "default");
    strcpy(env->templates[0].content, "Hello, {{name}}!");
}

Template* getTemplate(Environment* env, const char* templateName) {
    for (int i = 0; i < env->templateCount; i++) {
        if (strcmp(env->templateNames[i], templateName) == 0) {
            return &env->templates[i];
        }
    }
    return &env->templates[0];
}

void renderTemplate(Template* tmpl, const char* name, char* output) {
    char* pos = strstr(tmpl->content, "{{name}}");
    if (pos != NULL) {
        int prefixLen = pos - tmpl->content;
        strncpy(output, tmpl->content, prefixLen);
        output[prefixLen] = '\\0';
        strcat(output, name);
        strcat(output, pos + 8);
    } else {
        strcpy(output, tmpl->content);
    }
}

void initRequest(Request* request, const char* name) {
    if (name != NULL && strlen(name) > 0) {
        strcpy(request->name, name);
    } else {
        strcpy(request->name, "Guest");
    }
}

Response renderResponseFromEnv(Environment* env, Request* request) {
    Response response;
    Template* tmpl = getTemplate(env, "default");
    renderTemplate(tmpl, request->name, response.body);
    return response;
}

Response home(Environment* env, Request* request) {
    return renderResponseFromEnv(env, request);
}

int main() {
    Environment env;
    initEnvironment(&env);
    
    // Test case 1
    Request request1;
    initRequest(&request1, "John");
    Response response1 = home(&env, &request1);
    printf("Test 1: %s\\n", response1.body);
    
    // Test case 2
    Request request2;
    initRequest(&request2, "Alice");
    Response response2 = home(&env, &request2);
    printf("Test 2: %s\\n", response2.body);
    
    // Test case 3
    Request request3;
    initRequest(&request3, "Bob");
    Response response3 = home(&env, &request3);
    printf("Test 3: %s\\n", response3.body);
    
    // Test case 4 - No name parameter
    Request request4;
    initRequest(&request4, NULL);
    Response response4 = home(&env, &request4);
    printf("Test 4: %s\\n", response4.body);
    
    // Test case 5
    Request request5;
    initRequest(&request5, "Charlie");
    Response response5 = home(&env, &request5);
    printf("Test 5: %s\\n", response5.body);
    
    return 0;
}
