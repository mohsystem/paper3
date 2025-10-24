
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_RESOURCE_NAME 256
#define MAX_RESOURCE_DESC 512
#define MAX_RESOURCES 10

/* Structure to hold resource information */
typedef struct {
    char name[MAX_RESOURCE_NAME];
    char description[MAX_RESOURCE_DESC];
} Resource;

/* Resource access control structure */
typedef struct {
    Resource resources[MAX_RESOURCES];
    size_t count;
} ResourceAccessControl;

/* Initialize the resource access control with whitelisted resources */
void initResourceControl(ResourceAccessControl* rac) {
    if (rac == NULL) {
        return;
    }
    
    rac->count = 0;
    
    /* Add whitelisted resources with safe string copying */
    strncpy(rac->resources[rac->count].name, "document1", MAX_RESOURCE_NAME - 1);
    rac->resources[rac->count].name[MAX_RESOURCE_NAME - 1] = '\\0';
    strncpy(rac->resources[rac->count].description, 
            "Public document about services", MAX_RESOURCE_DESC - 1);
    rac->resources[rac->count].description[MAX_RESOURCE_DESC - 1] = '\\0';
    rac->count++;
    
    strncpy(rac->resources[rac->count].name, "document2", MAX_RESOURCE_NAME - 1);
    rac->resources[rac->count].name[MAX_RESOURCE_NAME - 1] = '\\0';
    strncpy(rac->resources[rac->count].description, 
            "User guide for application", MAX_RESOURCE_DESC - 1);
    rac->resources[rac->count].description[MAX_RESOURCE_DESC - 1] = '\\0';
    rac->count++;
    
    strncpy(rac->resources[rac->count].name, "report_2024", MAX_RESOURCE_NAME - 1);
    rac->resources[rac->count].name[MAX_RESOURCE_NAME - 1] = '\\0';
    strncpy(rac->resources[rac->count].description, 
            "Annual report data", MAX_RESOURCE_DESC - 1);
    rac->resources[rac->count].description[MAX_RESOURCE_DESC - 1] = '\\0';
    rac->count++;
    
    strncpy(rac->resources[rac->count].name, "config_template", MAX_RESOURCE_NAME - 1);
    rac->resources[rac->count].name[MAX_RESOURCE_NAME - 1] = '\\0';
    strncpy(rac->resources[rac->count].description, 
            "Configuration template", MAX_RESOURCE_DESC - 1);
    rac->resources[rac->count].description[MAX_RESOURCE_DESC - 1] = '\\0';
    rac->count++;
}

/* Validate resource name to prevent path traversal and injection attacks */
bool isValidResourceName(const char* resourceName) {
    size_t len;
    size_t i;
    
    if (resourceName == NULL) {
        return false;
    }
    
    len = strlen(resourceName);
    
    /* Check length bounds */
    if (len == 0 || len >= MAX_RESOURCE_NAME) {
        return false;
    }
    
    /* Reject names starting with dot (hidden files) */
    if (resourceName[0] == '.') {
        return false;
    }
    
    /* Check each character for validity */
    for (i = 0; i < len; i++) {
        char c = resourceName[i];
        
        /* Reject path separators */
        if (c == '/' || c == '\\\\' || c == ':') {\n            return false;\n        }\n        \n        /* Reject null bytes */\n        if (c == '\\0') {\n            break;\n        }\n        \n        /* Only allow alphanumeric, underscore, hyphen, and dot */\n        if (!isalnum((unsigned char)c) && c != '_' && c != '-' && c != '.') {\n            return false;\n        }\n    }\n    \n    /* Check for path traversal sequences */\n    if (strstr(resourceName, "..") != NULL) {\n        return false;\n    }\n    \n    return true;\n}\n\n/* Access resource with strict validation and whitelist checking */\nint accessResource(const ResourceAccessControl* rac, const char* resourceName, \n                   char* output, size_t outputSize) {\n    size_t i;\n    \n    if (rac == NULL || resourceName == NULL || output == NULL || outputSize == 0) {\n        return -1;\n    }\n    \n    /* Initialize output buffer */\n    output[0] = '\\0';\n    \n    /* Validate resource name format */\n    if (!isValidResourceName(resourceName)) {\n        snprintf(output, outputSize, "Error: Invalid resource name format");\n        return -1;\n    }\n    \n    /* Search for resource in whitelist */\n    for (i = 0; i < rac->count; i++) {\n        if (strcmp(rac->resources[i].name, resourceName) == 0) {\n            /* Resource found - return access granted message */\n            snprintf(output, outputSize, "Access granted to: %s - %s",\n                    rac->resources[i].name, rac->resources[i].description);\n            return 0;\n        }\n    }\n    \n    /* Resource not found in whitelist */\n    snprintf(output, outputSize, "Error: Resource not found or access denied");\n    return -1;\n}\n\n/* List all available resources */\nvoid listResources(const ResourceAccessControl* rac) {\n    size_t i;\n    \n    if (rac == NULL) {\n        return;\n    }\n    \n    printf("[Available Resources]:\\n");\n    for (i = 0; i < rac->count; i++) {\n        printf("  - %s\\n", rac->resources[i].name);\n    }\n}\n\n/* Safe input reading with bounds checking */\nbool safeReadLine(char* buffer, size_t bufferSize) {\n    char* newline;\n    \n    if (buffer == NULL || bufferSize == 0) {\n        return false;\n    }\n    \n    /* Read line with size limit */\n    if (fgets(buffer, (int)bufferSize, stdin) == NULL) {\n        buffer[0] = '\\0';\n        return false;\n    }\n    \n    /* Remove trailing newline */\n    newline = strchr(buffer, '\
');\n    if (newline != NULL) {\n        *newline = '\\0';\n    }\n    \n    /* Remove trailing whitespace */\n    while (strlen(buffer) > 0 && isspace((unsigned char)buffer[strlen(buffer) - 1])) {\n        buffer[strlen(buffer) - 1] = '\\0';\n    }\n    \n    return true;\n}\n\nint main(void) {\n    ResourceAccessControl rac;\n    char output[1024];\n    \n    /* Initialize with NULL checks */\n    memset(&rac, 0, sizeof(ResourceAccessControl));\n    memset(output, 0, sizeof(output));\n    \n    initResourceControl(&rac);\n    \n    printf("=== Resource Access Control System ===\\n");\n    \n    /* Test case 1: Valid resource access */\n    printf("\\n[Test 1] Accessing 'document1':\\n");\n    if (accessResource(&rac, "document1", output, sizeof(output)) == 0) {\n        printf("%s\\n", output);\n    } else {\n        printf("%s\\n", output);\n    }\n    \n    /* Test case 2: Another valid resource */\n    printf("\\n[Test 2] Accessing 'report_2024':\\n");\n    if (accessResource(&rac, "report_2024", output, sizeof(output)) == 0) {\n        printf("%s\\n", output);\n    } else {\n        printf("%s\\n", output);\n    }\n    \n    /* Test case 3: Path traversal attempt (blocked) */\n    printf("\\n[Test 3] Path traversal attempt '../etc/passwd':\\n");\n    accessResource(&rac, "../etc/passwd", output, sizeof(output));\n    printf("%s\\n", output);\n    \n    /* Test case 4: Non-existent resource */\n    printf("\\n[Test 4] Non-existent resource 'secret_data':\\n");\n    accessResource(&rac, "secret_data", output, sizeof(output));\n    printf("%s\\n", output);\n    \n    /* Test case 5: Invalid characters in resource name */\n    printf("\\n[Test 5] Invalid resource name 'doc/file':\
");\n    accessResource(&rac, "doc/file", output, sizeof(output));\n    printf("%s\
", output);\n    \n    /* List all available resources */\n    printf("\
");
    listResources(&rac);
    
    return 0;
}
