
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_STRING_LEN 256

typedef struct {
    char templateContent[MAX_STRING_LEN];
} Template;

typedef struct {
    char parameters[10][2][MAX_STRING_LEN];
    int paramCount;
} Request;

typedef struct {
    Request* request;
    Template templates[10];
    int templateCount;
} Environment;

typedef struct {
    char content[MAX_STRING_LEN];
} Response;

void Template_init(Template* tmpl, const char* content) {
    strncpy(tmpl->templateContent, content, MAX_STRING_LEN - 1);
    tmpl->templateContent[MAX_STRING_LEN - 1] = '\\0';
}

void Template_render(Template* tmpl, const char* key, const char* value, char* result) {
    char placeholder[MAX_STRING_LEN];
    snprintf(placeholder, MAX_STRING_LEN, "{{%s}}", key);
    
    char* pos = strstr(tmpl->templateContent, placeholder);
    if (pos != NULL) {
        int prefix_len = pos - tmpl->templateContent;
        strncpy(result, tmpl->templateContent, prefix_len);
        result[prefix_len] = '\\0';
        strcat(result, value);
        strcat(result, pos + strlen(placeholder));
    } else {
        strcpy(result, tmpl->templateContent);
    }
}

void Request_init(Request* req) {
    req->paramCount = 0;
}

void Request_addParameter(Request* req, const char* name, const char* value) {
    if (req->paramCount < 10) {
        strncpy(req->parameters[req->paramCount][0], name, MAX_STRING_LEN - 1);
        strncpy(req->parameters[req->paramCount][1], value, MAX_STRING_LEN - 1);
        req->paramCount++;
    }
}

const char* Request_getParameter(Request* req, const char* name) {
    for (int i = 0; i < req->paramCount; i++) {
        if (strcmp(req->parameters[i][0], name) == 0) {
            return req->parameters[i][1];
        }
    }
    return NULL;
}

void Environment_init(Environment* env, Request* req) {
    env->request = req;
    env->templateCount = 1;
    Template_init(&env->templates[0], "Hello, {{name}}!");
}

const char* Environment_getNameParameter(Environment* env) {
    return Request_getParameter(env->request, "name");
}

Template* Environment_getTemplate(Environment* env, const char* templateName) {
    return &env->templates[0];
}

void Response_init(Response* resp, const char* content) {
    strncpy(resp->content, content, MAX_STRING_LEN - 1);
    resp->content[MAX_STRING_LEN - 1] = '\\0';
}

Response renderResponseFromEnv(Environment* env) {
    Response resp;
    const char* name = Environment_getNameParameter(env);
    Template* tmpl = Environment_getTemplate(env, "default");
    
    char renderedContent[MAX_STRING_LEN];
    if (name == NULL) {
        Template_render(tmpl, "name", "World", renderedContent);
    } else {
        Template_render(tmpl, "name", name, renderedContent);
    }
    
    Response_init(&resp, renderedContent);
    return resp;
}

Response home(Environment* env) {
    return renderResponseFromEnv(env);
}

int main() {
    // Test case 1
    Request request1;
    Request_init(&request1);
    Request_addParameter(&request1, "name", "Alice");
    Environment env1;
    Environment_init(&env1, &request1);
    Response response1 = home(&env1);
    printf("Test 1: %s\\n", response1.content);
    
    // Test case 2
    Request request2;
    Request_init(&request2);
    Request_addParameter(&request2, "name", "Bob");
    Environment env2;
    Environment_init(&env2, &request2);
    Response response2 = home(&env2);
    printf("Test 2: %s\\n", response2.content);
    
    // Test case 3
    Request request3;
    Request_init(&request3);
    Request_addParameter(&request3, "name", "Charlie");
    Environment env3;
    Environment_init(&env3, &request3);
    Response response3 = home(&env3);
    printf("Test 3: %s\\n", response3.content);
    
    // Test case 4
    Request request4;
    Request_init(&request4);
    Request_addParameter(&request4, "name", "Diana");
    Environment env4;
    Environment_init(&env4, &request4);
    Response response4 = home(&env4);
    printf("Test 4: %s\\n", response4.content);
    
    // Test case 5
    Request request5;
    Request_init(&request5);
    Environment env5;
    Environment_init(&env5, &request5);
    Response response5 = home(&env5);
    printf("Test 5: %s\\n", response5.content);
    
    return 0;
}
