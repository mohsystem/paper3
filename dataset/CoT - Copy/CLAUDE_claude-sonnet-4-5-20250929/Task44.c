
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_INPUT_LENGTH 100

typedef struct {
    char data[MAX_INPUT_LENGTH + 1];
} SafeString;

void sanitizeInput(const char* input, SafeString* output) {
    if (input == NULL || output == NULL) {
        output->data[0] = '\\0';
        return;
    }
    
    int len = strlen(input);
    if (len > MAX_INPUT_LENGTH) {
        output->data[0] = '\\0';
        return;
    }
    
    int j = 0;
    for (int i = 0; i < len && j < MAX_INPUT_LENGTH; i++) {
        char c = input[i];
        // Skip dangerous characters
        if (c != '\\'' && c != '"' && c != ';' && c != '\\\\' && c != '<' && c != '>') {\n            output->data[j++] = c;\n        }\n    }\n    output->data[j] = '\\0';\n    \n    // Trim whitespace\n    while (j > 0 && isspace(output->data[j - 1])) {\n        output->data[--j] = '\\0';\n    }\n    \n    int start = 0;\n    while (output->data[start] && isspace(output->data[start])) {\n        start++;\n    }\n    if (start > 0) {\n        memmove(output->data, output->data + start, strlen(output->data + start) + 1);\n    }\n}\n\nbool validateAlphanumeric(const char* input) {\n    if (input == NULL || input[0] == '\\0') {\n        return false;\n    }\n    \n    for (int i = 0; input[i]; i++) {\n        if (!isalnum(input[i]) && !isspace(input[i])) {\n            return false;\n        }\n    }\n    return true;\n}\n\nbool validateEmail(const char* input) {\n    if (input == NULL || input[0] == '\\0' || strlen(input) > MAX_INPUT_LENGTH) {\n        return false;\n    }\n    \n    int atCount = 0;\n    int dotAfterAt = 0;\n    bool foundAt = false;\n    \n    for (int i = 0; input[i]; i++) {\n        if (input[i] == '@') {\n            atCount++;\n            foundAt = true;\n        } else if (foundAt && input[i] == '.') {\n            dotAfterAt++;\n        }\n    }\n    \n    return atCount == 1 && dotAfterAt >= 1;\n}\n\nbool validateNumeric(const char* input) {\n    if (input == NULL || input[0] == '\\0') {\n        return false;\n    }\n    \n    for (int i = 0; input[i]; i++) {\n        if (!isdigit(input[i])) {\n            return false;\n        }\n    }\n    return true;\n}\n\nint safeParseInt(const char* input, int defaultValue) {\n    if (!validateNumeric(input)) {\n        return defaultValue;\n    }\n    \n    long long value = atoll(input);\n    if (value > INT_MAX || value < INT_MIN) {\n        return defaultValue;\n    }\n    \n    return (int)value;\n}\n\nvoid processInput(const char* input, const char* inputType, char* output, size_t outputSize) {\n    if (input == NULL || inputType == NULL || output == NULL) {\n        snprintf(output, outputSize, "Error: Null input provided");\n        return;\n    }\n    \n    SafeString sanitized;\n    sanitizeInput(input, &sanitized);\n    \n    char typeLower[50];\n    strncpy(typeLower, inputType, sizeof(typeLower) - 1);\n    typeLower[sizeof(typeLower) - 1] = '\\0';\n    for (int i = 0; typeLower[i]; i++) {\n        typeLower[i] = tolower(typeLower[i]);\n    }\n    \n    if (strcmp(typeLower, "alphanumeric") == 0) {\n        if (validateAlphanumeric(sanitized.data)) {\n            snprintf(output, outputSize, "Valid alphanumeric input: %s", sanitized.data);\n        } else {\n            snprintf(output, outputSize, "Invalid alphanumeric input");\n        }\n    } else if (strcmp(typeLower, "email") == 0) {\n        if (validateEmail(sanitized.data)) {\n            snprintf(output, outputSize, "Valid email: %s", sanitized.data);\n        } else {\n            snprintf(output, outputSize, "Invalid email format");\n        }\n    } else if (strcmp(typeLower, "numeric") == 0) {\n        if (validateNumeric(sanitized.data)) {\n            int number = safeParseInt(sanitized.data, 0);\n            snprintf(output, outputSize, "Valid number: %d, Doubled: %d", number, number * 2);\n        } else {\n            snprintf(output, outputSize, "Invalid numeric input");\n        }\n    } else {\n        snprintf(output, outputSize, "Unknown input type");\n    }\n}\n\nint main() {\n    char output[500];\n    \n    printf("=== Secure Input Validation Test Cases ===\\n\\n");\n    \n    // Test Case 1: Valid alphanumeric input\n    printf("Test 1 - Valid Alphanumeric:\\n");\n    processInput("Hello World 123", "alphanumeric", output, sizeof(output));\n    printf("%s\\n\\n", output);\n    \n    // Test Case 2: Valid email\n    printf("Test 2 - Valid Email:\\n");\n    processInput("user@example.com", "email", output, sizeof(output));\n    printf("%s\\n\\n", output);\n    \n    // Test Case 3: Valid numeric input\n    printf("Test 3 - Valid Numeric:\\n");\n    processInput("42", "numeric", output, sizeof(output));\n    printf("%s\\n\\n", output);\n    \n    // Test Case 4: SQL Injection attempt\n    printf("Test 4 - SQL Injection Attempt:\\n");\n    processInput("admin' OR '1'='1", "alphanumeric", output, sizeof(output));\n    printf("%s\\n\\n", output);\n    \n    // Test Case 5: XSS attempt\n    printf("Test 5 - XSS Attempt:\\n");\n    processInput("<script>alert('XSS')</script>", "alphanumeric", output, sizeof(output));\n    printf("%s\
\
", output);
    
    return 0;
}
