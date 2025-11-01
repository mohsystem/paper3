
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_TEMPLATE_SIZE 1024
#define MAX_NAME_SIZE 256
#define MAX_OUTPUT_SIZE 2048

typedef struct {
    char content[MAX_OUTPUT_SIZE];
    int statusCode;
} Response;

typedef struct {
    char templates[10][MAX_TEMPLATE_SIZE];
    char templateNames[10][50];
    int templateCount;
} Environment;

void initEnvironment(Environment* env) {
    env->templateCount = 1;
    strcpy(env->templateNames[0], "default");
    strcpy(env->templates[0], "<html><body><h1>Hello, {{name}}!</h1></body></html>");
}

char* getTemplate(Environment* env, const char* templateName) {
    for (int i = 0; i < env->templateCount; i++) {
        if (strcmp(env->templateNames[i], templateName) == 0) {
            return env->templates[i];
        }
    }
    return env->templates[0];
}

void sanitizeInput(const char* input, char* output, size_t outputSize) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < outputSize - 10; i++) {
        switch (input[i]) {
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
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
                break;
        }
    }
    output[j] = '\\0';
}

void replaceString(char* str, const char* oldWord, const char* newWord) {
    char buffer[MAX_OUTPUT_SIZE];
    char* pos;
    
    if ((pos = strstr(str, oldWord)) != NULL) {
        strcpy(buffer, str);
        size_t index = pos - str;
        str[index] = '\\0';
        
        strcat(str, newWord);
        strcat(str, buffer + index + strlen(oldWord));
    }
}

Response renderResponseFromEnv(Environment* env, const char* name) {
    Response response;
    char sanitizedName[MAX_NAME_SIZE];
    
    // Get name parameter and sanitize
    if (name == NULL || strlen(name) == 0) {
        strcpy(sanitizedName, "Guest");
    } else {
        sanitizeInput(name, sanitizedName, MAX_NAME_SIZE);
    }
    
    // Get template from environment
    char* templateStr = getTemplate(env, "default");
    
    // Render template with name parameter
    strcpy(response.content, templateStr);
    replaceString(response.content, "{{name}}", sanitizedName);
    
    response.statusCode = 200;
    return response;
}

Response home(Environment* env, const char* name) {
    return renderResponseFromEnv(env, name);
}

int main() {
    Environment env;
    initEnvironment(&env);
    
    // Test case 1: Normal name
    Response response1 = home(&env, "John");
    printf("Test 1: %s\\n", response1.content);
    
    // Test case 2: Empty name (should default to Guest)
    Response response2 = home(&env, NULL);
    printf("Test 2: %s\\n", response2.content);
    
    // Test case 3: Name with special characters (XSS attempt)
    Response response3 = home(&env, "<script>alert('XSS')</script>");
    printf("Test 3: %s\\n", response3.content);
    
    // Test case 4: Name with quotes
    Response response4 = home(&env, "John \\"Doe\\"");
    printf("Test 4: %s\\n", response4.content);
    
    // Test case 5: Unicode name
    Response response5 = home(&env, "José");
    printf("Test 5: %s\\n", response5.content);
    
    return 0;
}
