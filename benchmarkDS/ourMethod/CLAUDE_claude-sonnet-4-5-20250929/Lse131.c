
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 100
#define MAX_TEMPLATE_LEN 256
#define MAX_OUTPUT_LEN 512

/* Environment structure holds templates */
typedef struct {
    char greeting_template[MAX_TEMPLATE_LEN];
    char welcome_template[MAX_TEMPLATE_LEN];
} Environment;

/* Initialize environment with safe templates */
void initEnvironment(Environment* env) {
    if (env == NULL) return;
    strncpy(env->greeting_template, "Hello, {{name}}!", MAX_TEMPLATE_LEN - 1);
    env->greeting_template[MAX_TEMPLATE_LEN - 1] = '\\0';
    strncpy(env->welcome_template, "Welcome, {{name}}!", MAX_TEMPLATE_LEN - 1);
    env->welcome_template[MAX_TEMPLATE_LEN - 1] = '\\0';
}

/* Validate template name: alphanumeric only, max 50 chars */
int isValidTemplateName(const char* name) {
    if (name == NULL || strlen(name) == 0 || strlen(name) > 50) {
        return 0;
    }
    
    for (size_t i = 0; name[i] != '\\0'; i++) {
        if (!isalnum((unsigned char)name[i]) && name[i] != '_') {
            return 0;
        }
    }
    return 1;
}

/* Get template from environment - returns NULL if not found */
const char* getTemplate(const Environment* env, const char* templateName) {
    if (env == NULL || !isValidTemplateName(templateName)) {
        return NULL;
    }
    
    if (strcmp(templateName, "greeting") == 0) {
        return env->greeting_template;
    } else if (strcmp(templateName, "welcome") == 0) {
        return env->welcome_template;
    }
    
    return NULL;
}

/* Safely escape HTML to prevent XSS - output buffer must be large enough */
void escapeHtml(const char* input, char* output, size_t outputSize) {
    if (input == NULL || output == NULL || outputSize == 0) {
        return;
    }
    
    size_t outIdx = 0;
    for (size_t i = 0; input[i] != '\\0' && outIdx < outputSize - 1; i++) {
        switch (input[i]) {
            case '&':
                if (outIdx + 5 < outputSize) {
                    memcpy(output + outIdx, "&amp;", 5);
                    outIdx += 5;
                }
                break;
            case '<':
                if (outIdx + 4 < outputSize) {
                    memcpy(output + outIdx, "&lt;", 4);
                    outIdx += 4;
                }
                break;
            case '>':
                if (outIdx + 4 < outputSize) {
                    memcpy(output + outIdx, "&gt;", 4);
                    outIdx += 4;
                }
                break;
            case '"':
                if (outIdx + 6 < outputSize) {
                    memcpy(output + outIdx, "&quot;", 6);
                    outIdx += 6;
                }
                break;
            case '\\'':
                if (outIdx + 6 < outputSize) {
                    memcpy(output + outIdx, "&#x27;", 6);
                    outIdx += 6;
                }
                break;
            case '/':
                if (outIdx + 6 < outputSize) {
                    memcpy(output + outIdx, "&#x2F;", 6);
                    outIdx += 6;
                }
                break;
            default:
                output[outIdx++] = input[i];
                break;
        }
    }
    output[outIdx] = '\\0';
}

/* Validate name: alphanumeric, spaces, hyphens only, max 100 chars */
void validateName(const char* input, char* output, size_t outputSize) {
    if (output == NULL || outputSize == 0) return;
    
    if (input == NULL || strlen(input) == 0) {
        strncpy(output, "Guest", outputSize - 1);
        output[outputSize - 1] = '\\0';
        return;
    }
    
    /* Trim and copy */
    size_t start = 0;
    while (input[start] != '\\0' && isspace((unsigned char)input[start])) {
        start++;
    }
    
    size_t len = strlen(input + start);
    if (len > MAX_NAME_LEN) {
        len = MAX_NAME_LEN;
    }
    
    size_t outIdx = 0;
    int valid = 1;
    for (size_t i = 0; i < len && input[start + i] != '\\0' && outIdx < outputSize - 1; i++) {
        char c = input[start + i];
        if (isalnum((unsigned char)c) || c == ' ' || c == '-') {
            output[outIdx++] = c;
        } else {
            valid = 0;
            break;
        }
    }
    
    if (!valid || outIdx == 0) {
        strncpy(output, "Guest", outputSize - 1);
        output[outputSize - 1] = '\\0';
    } else {
        output[outIdx] = '\\0';
    }
}

/* Render template with name - escapes output to prevent XSS */
void renderTemplate(const char* templateStr, const char* name, char* output, size_t outputSize) {
    if (templateStr == NULL || output == NULL || outputSize == 0) {
        return;
    }
    
    if (templateStr[0] == '\\0') {
        strncpy(output, "Invalid template", outputSize - 1);
        output[outputSize - 1] = '\\0';
        return;
    }
    
    /* Validate name */
    char safeName[MAX_NAME_LEN + 1];
    validateName(name, safeName, sizeof(safeName));
    
    /* Escape name for HTML */
    char escapedName[MAX_NAME_LEN * 6 + 1]; /* Worst case: all chars need escaping */
    escapeHtml(safeName, escapedName, sizeof(escapedName));
    
    /* Replace {{name}} with escaped name */
    const char* placeholder = "{{name}}";
    const char* pos = strstr(templateStr, placeholder);
    
    if (pos == NULL) {
        strncpy(output, templateStr, outputSize - 1);
        output[outputSize - 1] = '\\0';
        return;
    }
    
    size_t prefixLen = pos - templateStr;
    size_t nameLen = strlen(escapedName);
    size_t suffixLen = strlen(pos + strlen(placeholder));
    
    /* Check buffer bounds */
    if (prefixLen + nameLen + suffixLen >= outputSize) {
        strncpy(output, "Output buffer too small", outputSize - 1);
        output[outputSize - 1] = '\\0';
        return;
    }
    
    /* Copy parts */
    memcpy(output, templateStr, prefixLen);
    memcpy(output + prefixLen, escapedName, nameLen);
    memcpy(output + prefixLen + nameLen, pos + strlen(placeholder), suffixLen);
    output[prefixLen + nameLen + suffixLen] = '\\0';
}

/* Extract parameter from query string */
void getParameter(const char* queryString, const char* paramName, char* output, size_t outputSize) {
    if (queryString == NULL || paramName == NULL || output == NULL || outputSize == 0) {
        if (output != NULL && outputSize > 0) output[0] = '\\0';
        return;
    }
    
    output[0] = '\\0';
    
    char searchStr[64];
    snprintf(searchStr, sizeof(searchStr), "%s=", paramName);
    
    const char* pos = strstr(queryString, searchStr);
    if (pos == NULL) {
        return;
    }
    
    pos += strlen(searchStr);
    const char* endPos = strchr(pos, '&');
    size_t len;
    
    if (endPos == NULL) {
        len = strlen(pos);
    } else {
        len = endPos - pos;
    }
    
    if (len >= outputSize) {
        len = outputSize - 1;
    }
    
    memcpy(output, pos, len);
    output[len] = '\\0';
}

/* Render response from environment - secure implementation */
void renderResponseFromEnv(const Environment* env, const char* requestData, char* output, size_t outputSize) {
    if (env == NULL || output == NULL || outputSize == 0) {
        return;
    }
    
    /* Get name parameter from request */
    char name[MAX_NAME_LEN + 1];
    getParameter(requestData, "name", name, sizeof(name));
    
    /* Get template from environment */
    const char* templateStr = getTemplate(env, "greeting");
    if (templateStr == NULL) {
        strncpy(output, "Error: Template not found", outputSize - 1);
        output[outputSize - 1] = '\\0';
        return;
    }
    
    /* Render template */
    renderTemplate(templateStr, name, output, outputSize);
}

/* Home handler that calls renderResponseFromEnv */
void home(const Environment* env, const char* requestData, char* output, size_t outputSize) {
    renderResponseFromEnv(env, requestData, output, outputSize);
}

int main(void) {
    Environment env;
    initEnvironment(&env);
    
    char output[MAX_OUTPUT_LEN];
    
    /* Test cases */
    printf("Test Case 1 (valid name):\\n");
    home(&env, "name=Alice", output, sizeof(output));
    printf("%s\\n", output);
    
    printf("\\nTest Case 2 (XSS attempt):\\n");
    home(&env, "name=<script>alert('XSS')</script>", output, sizeof(output));
    printf("%s\\n", output);
    
    printf("\\nTest Case 3 (no name parameter):\\n");
    home(&env, "", output, sizeof(output));
    printf("%s\\n", output);
    
    printf("\\nTest Case 4 (valid name with spaces):\\n");
    home(&env, "name=John Doe", output, sizeof(output));
    printf("%s\\n", output);
    
    printf("\\nTest Case 5 (special characters):\\n");
    home(&env, "name=Test&User", output, sizeof(output));
    printf("%s\\n", output);
    
    return 0;
}
