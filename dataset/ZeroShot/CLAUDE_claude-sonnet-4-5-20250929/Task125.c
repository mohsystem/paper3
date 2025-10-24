
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_RESOURCES 5
#define MAX_STRING_LENGTH 256

const char* ALLOWED_RESOURCES[MAX_RESOURCES] = {
    "document1.txt", "document2.txt", "image1.jpg", "image2.jpg", "data.csv"
};

void trim(char* str) {
    if (str == NULL) return;
    
    int start = 0;
    int end = strlen(str) - 1;
    
    while (isspace((unsigned char)str[start])) start++;
    while (end >= start && isspace((unsigned char)str[end])) end--;
    
    int i;
    for (i = 0; i <= end - start; i++) {
        str[i] = str[start + i];
    }
    str[i] = '\\0';
}

bool isValidResourceFormat(const char* resourceName) {
    if (resourceName == NULL || strlen(resourceName) == 0) {
        return false;
    }
    
    for (int i = 0; resourceName[i] != '\\0'; i++) {
        char c = resourceName[i];
        if (!isalnum(c) && c != '.' && c != '_' && c != '-') {
            return false;
        }
    }
    return true;
}

bool containsPathTraversal(const char* resourceName) {
    return strstr(resourceName, "..") != NULL ||
           strchr(resourceName, '/') != NULL ||
           strchr(resourceName, '\\\\') != NULL;\n}\n\nbool isResourceAllowed(const char* resourceName) {\n    for (int i = 0; i < MAX_RESOURCES; i++) {\n        if (strcmp(resourceName, ALLOWED_RESOURCES[i]) == 0) {\n            return true;\n        }\n    }\n    return false;\n}\n\nvoid accessResource(const char* username, const char* resourceName, char* result) {\n    char usernameCopy[MAX_STRING_LENGTH];\n    char resourceCopy[MAX_STRING_LENGTH];\n    \n    // Copy and validate username\n    if (username == NULL || strlen(username) == 0) {\n        strcpy(result, "Error: Invalid username");\n        return;\n    }\n    strncpy(usernameCopy, username, MAX_STRING_LENGTH - 1);\n    usernameCopy[MAX_STRING_LENGTH - 1] = '\\0';\n    trim(usernameCopy);\n    \n    if (strlen(usernameCopy) == 0) {\n        strcpy(result, "Error: Invalid username");\n        return;\n    }\n    \n    // Copy and validate resource name\n    if (resourceName == NULL || strlen(resourceName) == 0) {\n        strcpy(result, "Error: Invalid resource name");\n        return;\n    }\n    strncpy(resourceCopy, resourceName, MAX_STRING_LENGTH - 1);\n    resourceCopy[MAX_STRING_LENGTH - 1] = '\\0';\n    trim(resourceCopy);\n    \n    if (strlen(resourceCopy) == 0) {\n        strcpy(result, "Error: Invalid resource name");\n        return;\n    }\n    \n    // Validate resource format\n    if (!isValidResourceFormat(resourceCopy)) {\n        strcpy(result, "Error: Invalid resource name format");\n        return;\n    }\n    \n    // Check for path traversal\n    if (containsPathTraversal(resourceCopy)) {\n        strcpy(result, "Error: Path traversal attempt detected");\n        return;\n    }\n    \n    // Check whitelist\n    if (!isResourceAllowed(resourceCopy)) {\n        strcpy(result, "Error: Access denied - resource not found or unauthorized");\n        return;\n    }\n    \n    // Success\n    snprintf(result, MAX_STRING_LENGTH, "Success: User '%s' accessed resource '%s'", \n             usernameCopy, resourceCopy);\n}\n\nint main() {\n    char result[MAX_STRING_LENGTH];\n    \n    printf("=== Resource Access Control System ===\
\
");\n    \n    // Test case 1: Valid access\n    printf("Test 1 - Valid access:\
");\n    accessResource("john_doe", "document1.txt", result);\n    printf("%s\
\
", result);\n    \n    // Test case 2: Valid access to different resource\n    printf("Test 2 - Valid access to image:\
");\n    accessResource("jane_smith", "image1.jpg", result);\n    printf("%s\
\
", result);\n    \n    // Test case 3: Attempt to access non-existent resource\n    printf("Test 3 - Non-existent resource:\
");\n    accessResource("user1", "secret.txt", result);\n    printf("%s\
\
", result);\n    \n    // Test case 4: Path traversal attempt\n    printf("Test 4 - Path traversal attempt:\
");\n    accessResource("attacker", "../../../etc/passwd", result);\n    printf("%s\
\
", result);\n    \n    // Test case 5: Invalid input\n    printf("Test 5 - Invalid input:\
");\n    accessResource("", "document1.txt", result);\n    printf("%s\
", result);\n    accessResource("user2", "", result);\n    printf("%s\
\
", result);
    
    return 0;
}
