
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_TEMPLATES 10
#define MAX_TEMPLATE_NAME 100
#define MAX_TEMPLATE_CONTENT 1000
#define MAX_URI_LENGTH 2000
#define MAX_PARAM_LENGTH 1000
#define MAX_RESPONSE_LENGTH 2000

/* Template structure */
typedef struct {
    char name[MAX_TEMPLATE_NAME];
    char content[MAX_TEMPLATE_CONTENT];
} TemplateEntry;

/* Environment structure that stores templates */
typedef struct {
    TemplateEntry templates[MAX_TEMPLATES];
    int count;
} Environment;

/* Response structure */
typedef struct {
    char body[MAX_RESPONSE_LENGTH];
    int statusCode;
} Response;

/* Initialize environment - all pointers set to NULL or valid memory */
void initEnvironment(Environment* env) {
    if (env == NULL) return;
    memset(env, 0, sizeof(Environment)); /* Zero all memory */
    env->count = 0;
}

/* Add template to environment with bounds checking */
bool addTemplate(Environment* env, const char* name, const char* content) {
    if (env == NULL || name == NULL || content == NULL) {
        return false;
    }
    
    /* Check bounds before adding */
    if (env->count >= MAX_TEMPLATES) {
        return false;
    }
    
    /* Validate lengths to prevent buffer overflow */
    size_t nameLen = strnlen(name, MAX_TEMPLATE_NAME);
    size_t contentLen = strnlen(content, MAX_TEMPLATE_CONTENT);
    
    if (nameLen == 0 || nameLen >= MAX_TEMPLATE_NAME - 1 ||
        contentLen == 0 || contentLen >= MAX_TEMPLATE_CONTENT - 1) {
        return false;
    }
    
    /* Safe copy with explicit null termination */
    strncpy(env->templates[env->count].name, name, MAX_TEMPLATE_NAME - 1);
    env->templates[env->count].name[MAX_TEMPLATE_NAME - 1] = '\\0';
    
    strncpy(env->templates[env->count].content, content, MAX_TEMPLATE_CONTENT - 1);
    env->templates[env->count].content[MAX_TEMPLATE_CONTENT - 1] = '\\0';
    
    env->count++;
    return true;
}

/* Get template from environment */
const char* getTemplate(const Environment* env, const char* name) {
    if (env == NULL || name == NULL) {
        return NULL;
    }
    
    /* Search for template with bounds checking */
    for (int i = 0; i < env->count && i < MAX_TEMPLATES; i++) {
        if (strncmp(env->templates[i].name, name, MAX_TEMPLATE_NAME) == 0) {
            return env->templates[i].content;
        }
    }
    return NULL;
}

/* Validate name matches safe pattern - alphanumeric, underscore, hyphen, space only */
bool isValidName(const char* name) {
    if (name == NULL) return false;
    
    size_t len = strnlen(name, MAX_PARAM_LENGTH + 1);
    if (len == 0 || len > 100) return false;
    
    /* Check each character is in allowed set */
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (!isalnum((unsigned char)c) && c != '_' && c != '-' && c != ' ') {
            return false;
        }
    }
    return true;
}

/* Escape HTML special characters to prevent XSS - writes to dest with bounds check */
void escapeHtml(const char* src, char* dest, size_t destSize) {
    if (src == NULL || dest == NULL || destSize == 0) return;
    
    size_t srcLen = strnlen(src, MAX_PARAM_LENGTH);
    size_t destIdx = 0;
    
    for (size_t i = 0; i < srcLen && destIdx < destSize - 1; i++) {
        const char* replacement = NULL;
        size_t repLen = 0;
        
        /* Determine replacement string for special characters */
        switch (src[i]) {
            case '&': replacement = "&amp;"; repLen = 5; break;
            case '<': replacement = "&lt;"; repLen = 4; break;
            case '>': replacement = "&gt;"; repLen = 4; break;
            case '"': replacement = "&quot;"; repLen = 6; break;
            case '\\'': replacement = "&#x27;"; repLen = 6; break;
            case '/': replacement = "&#x2F;"; repLen = 6; break;
            default: 
                dest[destIdx++] = src[i];
                continue;
        }
        
        /* Check bounds before copying replacement */
        if (destIdx + repLen < destSize) {
            memcpy(dest + destIdx, replacement, repLen);
            destIdx += repLen;
        } else {
            break; /* Not enough space */
        }
    }
    
    /* Ensure null termination */
    dest[destIdx] = '\\0';
}

/* URL decode with bounds checking */
int hexToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

void urlDecode(const char* src, char* dest, size_t destSize) {
    if (src == NULL || dest == NULL || destSize == 0) return;
    
    size_t srcLen = strnlen(src, MAX_URI_LENGTH);
    size_t destIdx = 0;
    
    for (size_t i = 0; i < srcLen && destIdx < destSize - 1; i++) {
        if (src[i] == '%' && i + 2 < srcLen) {
            int high = hexToInt(src[i + 1]);
            int low = hexToInt(src[i + 2]);
            if (high >= 0 && low >= 0) {
                dest[destIdx++] = (char)((high << 4) | low);
                i += 2;
            } else {
                dest[destIdx++] = src[i];
            }
        } else if (src[i] == '+') {
            dest[destIdx++] = ' ';
        } else {
            dest[destIdx++] = src[i];
        }
    }
    
    /* Ensure null termination */
    dest[destIdx] = '\\0';
}

/* Extract query parameter safely from URI with bounds checking */
bool getQueryParameter(const char* requestUri, const char* paramName, char* result, size_t resultSize) {
    if (requestUri == NULL || paramName == NULL || result == NULL || resultSize == 0) {
        return false;
    }
    
    /* Initialize result */
    result[0] = '\\0';
    
    /* Find query string start */
    const char* queryStart = strchr(requestUri, '?');
    if (queryStart == NULL) {
        return false;
    }
    queryStart++; /* Skip '?' */
    
    /* Parse query parameters with bounds checking */
    char* queryCopy = strndup(queryStart, MAX_URI_LENGTH);
    if (queryCopy == NULL) return false;
    
    char* saveptr = NULL;
    char* pair = strtok_r(queryCopy, "&", &saveptr);
    bool found = false;
    
    while (pair != NULL) {
        char* eqPos = strchr(pair, '=');
        if (eqPos != NULL) {
            *eqPos = '\\0';
            char* key = pair;
            char* value = eqPos + 1;
            
            /* Check if this is our parameter */
            if (strncmp(key, paramName, MAX_TEMPLATE_NAME) == 0) {
                /* URL decode the value with bounds checking */
                char decoded[MAX_PARAM_LENGTH];
                urlDecode(value, decoded, sizeof(decoded));
                
                /* Limit parameter length to prevent DoS */
                if (strnlen(decoded, MAX_PARAM_LENGTH + 1) <= MAX_PARAM_LENGTH) {
                    strncpy(result, decoded, resultSize - 1);
                    result[resultSize - 1] = '\\0';
                    found = true;
                }
                break;
            }
        }
        pair = strtok_r(NULL, "&", &saveptr);
    }
    
    free(queryCopy); /* Free allocated memory exactly once */
    return found;
}

/* Render template with validated name parameter */
void renderTemplate(const char* templateContent, const char* name, char* result, size_t resultSize) {
    if (templateContent == NULL || result == NULL || resultSize == 0) return;
    
    char safeName[MAX_PARAM_LENGTH];
    
    /* Validate name parameter - use default if invalid */
    if (name == NULL || !isValidName(name)) {
        strncpy(safeName, "Guest", sizeof(safeName) - 1);
        safeName[sizeof(safeName) - 1] = '\\0';
    } else {
        strncpy(safeName, name, sizeof(safeName) - 1);
        safeName[sizeof(safeName) - 1] = '\\0';
    }
    
    /* Escape HTML to prevent XSS */
    char escapedName[MAX_PARAM_LENGTH * 6]; /* Max expansion for HTML entities */
    escapeHtml(safeName, escapedName, sizeof(escapedName));
    
    /* Replace {{name}} with escaped name - simple implementation with bounds checking */
    const char* placeholder = "{{name}}";
    const char* pos = strstr(templateContent, placeholder);
    
    if (pos != NULL) {
        size_t beforeLen = pos - templateContent;
        size_t afterPos = beforeLen + strlen(placeholder);
        size_t escapedLen = strnlen(escapedName, sizeof(escapedName));
        size_t afterLen = strnlen(templateContent + afterPos, MAX_TEMPLATE_CONTENT);
        
        /* Check total length won't overflow result buffer */\n        if (beforeLen + escapedLen + afterLen < resultSize) {\n            /* Copy before placeholder */\n            memcpy(result, templateContent, beforeLen);\n            /* Copy escaped name */\n            memcpy(result + beforeLen, escapedName, escapedLen);\n            /* Copy after placeholder */\n            memcpy(result + beforeLen + escapedLen, templateContent + afterPos, afterLen);\n            /* Null terminate */\n            result[beforeLen + escapedLen + afterLen] = '\\0';\n        } else {\n            /* Fallback: just copy template as-is if result would overflow */\n            strncpy(result, templateContent, resultSize - 1);\n            result[resultSize - 1] = '\\0';\n        }\n    } else {\n        /* No placeholder found, copy template as-is */\n        strncpy(result, templateContent, resultSize - 1);\n        result[resultSize - 1] = '\\0';\n    }\n}\n\n/* Render response from environment with security controls */\nResponse renderResponseFromEnv(const Environment* env, const char* requestUri) {\n    Response response;\n    memset(&response, 0, sizeof(Response)); /* Initialize all fields */\n    \n    if (env == NULL || requestUri == NULL) {\n        strncpy(response.body, "Bad Request", sizeof(response.body) - 1);\n        response.body[sizeof(response.body) - 1] = '\\0';\n        response.statusCode = 400;\n        return response;\n    }\n    \n    /* Get name parameter from request with input validation */\n    char name[MAX_PARAM_LENGTH];\n    getQueryParameter(requestUri, "name", name, sizeof(name));\n    \n    /* Get template from environment - use fixed name to prevent template injection */\n    const char* templateContent = getTemplate(env, "greeting");\n    if (templateContent == NULL) {\n        strncpy(response.body, "Template not found", sizeof(response.body) - 1);\n        response.body[sizeof(response.body) - 1] = '\\0';
        response.statusCode = 500;
        return response;
    }
    
    /* Render template with validated name */
    renderTemplate(templateContent, name[0] ? name : NULL, response.body, sizeof(response.body));
    response.statusCode = 200;
    
    return response;
}

/* Home function that calls renderResponseFromEnv */
Response home(const Environment* env, const char* requestUri) {
    /* Delegate to renderResponseFromEnv */
    return renderResponseFromEnv(env, requestUri);
}

/* Test cases */
int main(void) {
    Environment env;
    initEnvironment(&env);
    addTemplate(&env, "greeting", "<html><body>Hello, {{name}}!</body></html>");
    
    printf("Test Case 1: Valid name parameter\\n");
    Response resp1 = home(&env, "http://localhost/?name=Alice");
    printf("Status: %d\\n", resp1.statusCode);
    printf("Body: %s\\n\\n", resp1.body);
    
    printf("Test Case 2: Name with spaces\\n");
    Response resp2 = home(&env, "http://localhost/?name=John+Doe");
    printf("Status: %d\\n", resp2.statusCode);
    printf("Body: %s\\n\\n", resp2.body);
    
    printf("Test Case 3: XSS attempt (should be escaped)\\n");
    Response resp3 = home(&env, "http://localhost/?name=%3Cscript%3Ealert(1)%3C%2Fscript%3E");
    printf("Status: %d\\n", resp3.statusCode);
    printf("Body: %s\\n\\n", resp3.body);
    
    printf("Test Case 4: No name parameter (should use default)\\n");
    Response resp4 = home(&env, "http://localhost/");
    printf("Status: %d\\n", resp4.statusCode);
    printf("Body: %s\\n\\n", resp4.body);
    
    printf("Test Case 5: Invalid characters (should use default)\\n");
    Response resp5 = home(&env, "http://localhost/?name=%00%01%02");
    printf("Status: %d\\n", resp5.statusCode);
    printf("Body: %s\\n", resp5.body);
    
    return 0;
}
