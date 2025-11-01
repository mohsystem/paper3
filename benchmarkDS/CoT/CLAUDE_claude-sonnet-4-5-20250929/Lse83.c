
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_PAYLOAD_SIZE 10240
#define MAX_NAME_LENGTH 100

typedef struct {
    char result[512];
    int status;
} PayloadResult;

bool isValidNameChar(char c) {
    return isalnum(c) || c == ' ' || c == '-' || c == '_' || c == '.';
}

bool validateName(const char* name) {
    if (name == NULL || strlen(name) == 0 || strlen(name) > MAX_NAME_LENGTH) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(name); i++) {
        if (!isValidNameChar(name[i])) {
            return false;
        }
    }
    return true;
}

void escapeJson(const char* input, char* output, size_t outputSize) {
    size_t j = 0;
    for (size_t i = 0; i < strlen(input) && j < outputSize - 2; i++) {
        switch (input[i]) {
            case '\\\\': \n                if (j < outputSize - 3) { output[j++] = '\\\\'; output[j++] = '\\\\'; }\n                break;\n            case '"':\n                if (j < outputSize - 3) { output[j++] = '\\\\'; output[j++] = '"'; }\n                break;\n            case '\
':\n                if (j < outputSize - 3) { output[j++] = '\\\\'; output[j++] = 'n'; }\n                break;\n            case '\\r':\n                if (j < outputSize - 3) { output[j++] = '\\\\'; output[j++] = 'r'; }\n                break;\n            case '\\t':\n                if (j < outputSize - 3) { output[j++] = '\\\\'; output[j++] = 't'; }\n                break;\n            default:\n                output[j++] = input[i];\n                break;\n        }\n    }\n    output[j] = '\\0';\n}\n\nPayloadResult processPayload(const char* jsonPayload) {\n    PayloadResult result;\n    result.status = 400;\n    \n    if (jsonPayload == NULL || strlen(jsonPayload) == 0) {\n        strcpy(result.result, "{\\"error\\": \\"Empty payload\\"}");\n        return result;\n    }\n    \n    if (strlen(jsonPayload) > MAX_PAYLOAD_SIZE) {\n        strcpy(result.result, "{\\"error\\": \\"Payload too large\\"}");\n        return result;\n    }\n    \n    // Simple JSON parsing for "name" field\n    const char* nameKey = strstr(jsonPayload, "\\"name\\"");\n    if (nameKey == NULL) {\n        strcpy(result.result, "{\\"error\\": \\"Name field not found\\"}");\n        return result;\n    }\n    \n    // Find the value after "name":\n    const char* colonPos = strchr(nameKey, ':');\n    if (colonPos == NULL) {\n        strcpy(result.result, "{\\"error\\": \\"Invalid JSON format\\"}");\n        return result;\n    }\n    \n    // Skip whitespace and opening quote\n    const char* valueStart = colonPos + 1;\n    while (*valueStart == ' ' || *valueStart == '\\t') valueStart++;\n    \n    if (*valueStart != '"') {\n        strcpy(result.result, "{\\"error\\": \\"Invalid name format\\"}");
        return result;
    }
    valueStart++;
    
    // Find closing quote
    const char* valueEnd = strchr(valueStart, '"');
    if (valueEnd == NULL) {
        strcpy(result.result, "{\\"error\\": \\"Invalid JSON format\\"}");
        return result;
    }
    
    // Extract name
    size_t nameLen = valueEnd - valueStart;
    if (nameLen == 0 || nameLen > MAX_NAME_LENGTH) {
        strcpy(result.result, "{\\"error\\": \\"Name field is empty or too long\\"}");
        return result;
    }
    
    char name[MAX_NAME_LENGTH + 1];
    strncpy(name, valueStart, nameLen);
    name[nameLen] = '\\0';
    
    // Validate name
    if (!validateName(name)) {
        strcpy(result.result, "{\\"error\\": \\"Invalid name format\\"}");
        return result;
    }
    
    // Escape and return name
    char escapedName[MAX_NAME_LENGTH * 2 + 1];
    escapeJson(name, escapedName, sizeof(escapedName));
    
    snprintf(result.result, sizeof(result.result), "{\\"name\\": \\"%s\\"}", escapedName);
    result.status = 200;
    
    return result;
}

int main() {
    // Test cases
    PayloadResult r1 = processPayload("{\\"name\\": \\"John Doe\\"}");
    printf("Test Case 1: %s (Status: %d)\\n", r1.result, r1.status);
    
    PayloadResult r2 = processPayload("{\\"age\\": 30}");
    printf("Test Case 2: %s (Status: %d)\\n", r2.result, r2.status);
    
    PayloadResult r3 = processPayload("{\\"name\\": \\"\\"}");
    printf("Test Case 3: %s (Status: %d)\\n", r3.result, r3.status);
    
    PayloadResult r4 = processPayload("{}");
    printf("Test Case 4: %s (Status: %d)\\n", r4.result, r4.status);
    
    PayloadResult r5 = processPayload("{\\"name\\": \\"Alice_123\\"}");
    printf("Test Case 5: %s (Status: %d)\\n", r5.result, r5.status);
    
    return 0;
}
