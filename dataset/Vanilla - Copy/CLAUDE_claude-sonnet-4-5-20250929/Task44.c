
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_INPUT_LENGTH 100
#define MAX_OUTPUT_LENGTH 256

bool is_alphanumeric(const char* str) {
    for (int i = 0; str[i] != '\\0'; i++) {
        if (!isalnum(str[i]) && str[i] != ' ') {
            return false;
        }
    }
    return true;
}

bool is_numeric(const char* str) {
    int i = 0;
    if (str[0] == '-') i = 1;
    
    if (str[i] == '\\0') return false;
    
    for (; str[i] != '\\0'; i++) {
        if (!isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

bool is_valid_email(const char* str) {
    int at_count = 0;
    int dot_after_at = 0;
    bool found_at = false;
    
    for (int i = 0; str[i] != '\\0'; i++) {
        if (str[i] == '@') {
            at_count++;
            found_at = true;
            if (i == 0 || str[i+1] == '\\0') return false;
        } else if (found_at && str[i] == '.') {
            dot_after_at++;
        }
    }
    
    return (at_count == 1 && dot_after_at > 0);
}

void validate_and_sanitize_input(const char* input, char* output) {
    if (input == NULL || strlen(input) == 0) {
        strcpy(output, "Error: Input cannot be null or empty");
        return;
    }
    
    if (strlen(input) > MAX_INPUT_LENGTH) {
        sprintf(output, "Error: Input exceeds maximum length of %d", MAX_INPUT_LENGTH);
        return;
    }
    
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < MAX_OUTPUT_LENGTH - 1; i++) {
        char c = input[i];
        if (c != '<' && c != '>' && c != '"' && c != '\\'' && 
            c != ';' && c != '(' && c != ')' && c != '{' && 
            c != '}' && c != '[' && c != ']' && c != '\\\\') {\n            output[j++] = c;\n        }\n    }\n    output[j] = '\\0';\n    \n    // Trim whitespace\n    while (j > 0 && isspace(output[j-1])) {\n        output[--j] = '\\0';\n    }\n}\n\nvoid validate_alphanumeric(const char* input, char* output) {\n    char sanitized[MAX_OUTPUT_LENGTH];\n    validate_and_sanitize_input(input, sanitized);\n    \n    if (strncmp(sanitized, "Error:", 6) == 0) {\n        strcpy(output, sanitized);\n        return;\n    }\n    \n    if (!is_alphanumeric(sanitized)) {\n        strcpy(output, "Error: Input must contain only alphanumeric characters and spaces");\n        return;\n    }\n    \n    sprintf(output, "Valid alphanumeric: %s", sanitized);\n}\n\nvoid validate_email(const char* input, char* output) {\n    char sanitized[MAX_OUTPUT_LENGTH];\n    validate_and_sanitize_input(input, sanitized);\n    \n    if (strncmp(sanitized, "Error:", 6) == 0) {\n        strcpy(output, sanitized);\n        return;\n    }\n    \n    if (!is_valid_email(sanitized)) {\n        strcpy(output, "Error: Invalid email format");\n        return;\n    }\n    \n    sprintf(output, "Valid email: %s", sanitized);\n}\n\nvoid validate_numeric(const char* input, char* output) {\n    char sanitized[MAX_OUTPUT_LENGTH];\n    validate_and_sanitize_input(input, sanitized);\n    \n    if (strncmp(sanitized, "Error:", 6) == 0) {\n        strcpy(output, sanitized);\n        return;\n    }\n    \n    if (!is_numeric(sanitized)) {\n        strcpy(output, "Error: Input must be a valid number");\n        return;\n    }\n    \n    sprintf(output, "Valid number: %s", sanitized);\n}\n\nvoid process_input(const char* input, const char* type, char* output) {\n    if (type == NULL || strlen(type) == 0) {\n        strcpy(output, "Error: Validation type not specified");\n        return;\n    }\n    \n    char lower_type[50];\n    strcpy(lower_type, type);\n    for (int i = 0; lower_type[i]; i++) {\n        lower_type[i] = tolower(lower_type[i]);\n    }\n    \n    if (strcmp(lower_type, "alphanumeric") == 0) {\n        validate_alphanumeric(input, output);\n    } else if (strcmp(lower_type, "email") == 0) {\n        validate_email(input, output);\n    } else if (strcmp(lower_type, "numeric") == 0) {\n        validate_numeric(input, output);\n    } else {\n        strcpy(output, "Error: Unknown validation type");\n    }\n}\n\nint main() {\n    char output[MAX_OUTPUT_LENGTH];\n    \n    printf("=== Test Case 1: Valid Alphanumeric ===\\n");\n    process_input("Hello World 123", "alphanumeric", output);\n    printf("%s\\n", output);\n    \n    printf("\\n=== Test Case 2: Valid Email ===\\n");\n    process_input("user@example.com", "email", output);\n    printf("%s\\n", output);\n    \n    printf("\\n=== Test Case 3: Valid Numeric ===\\n");\n    process_input("12345", "numeric", output);\n    printf("%s\\n", output);\n    \n    printf("\\n=== Test Case 4: Invalid Input with Special Characters ===\\n");\n    process_input("<script>alert('xss')</script>", "alphanumeric", output);\n    printf("%s\\n", output);\n    \n    printf("\\n=== Test Case 5: Input Too Long ===\\n");\n    char long_input[200];\n    memset(long_input, 'a', 150);\n    long_input[150] = '\\0';
    process_input(long_input, "alphanumeric", output);
    printf("%s\\n", output);
    
    return 0;
}
