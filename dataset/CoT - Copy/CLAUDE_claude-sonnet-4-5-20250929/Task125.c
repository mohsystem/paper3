
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_USERNAME_LEN 21
#define MAX_RESOURCE_LEN 51
#define MAX_CONTENT_LEN 256
#define MAX_USERS 3
#define MAX_RESOURCES_PER_USER 2

typedef struct {
    char name[MAX_RESOURCE_LEN];
    char content[MAX_CONTENT_LEN];
} Resource;

typedef struct {
    char username[MAX_USERNAME_LEN];
    Resource resources[MAX_RESOURCES_PER_USER];
    int resourceCount;
} User;

static User users[MAX_USERS];
static int userCount = 0;

void initializeResources() {
    static bool initialized = false;
    if (initialized) return;
    
    // User 1
    strcpy(users[0].username, "user1");
    strcpy(users[0].resources[0].name, "document1");
    strcpy(users[0].resources[0].content, "Content of Document 1 for User1");
    strcpy(users[0].resources[1].name, "report2");
    strcpy(users[0].resources[1].content, "Annual Report Data for User1");
    users[0].resourceCount = 2;
    
    // User 2
    strcpy(users[1].username, "user2");
    strcpy(users[1].resources[0].name, "file3");
    strcpy(users[1].resources[0].content, "Private File Content for User2");
    strcpy(users[1].resources[1].name, "data4");
    strcpy(users[1].resources[1].content, "Database Export for User2");
    users[1].resourceCount = 2;
    
    // Admin
    strcpy(users[2].username, "admin");
    strcpy(users[2].resources[0].name, "config");
    strcpy(users[2].resources[0].content, "System Configuration Data");
    strcpy(users[2].resources[1].name, "logs");
    strcpy(users[2].resources[1].content, "System Logs and Monitoring");
    users[2].resourceCount = 2;
    
    userCount = 3;
    initialized = true;
}

void trimString(char* str) {
    if (str == NULL) return;
    
    // Trim leading spaces
    int start = 0;
    while (str[start] && isspace((unsigned char)str[start])) start++;
    
    // Trim trailing spaces
    int end = strlen(str) - 1;
    while (end >= 0 && isspace((unsigned char)str[end])) end--;
    
    // Move trimmed string to beginning
    int len = end - start + 1;
    if (len > 0) {
        memmove(str, str + start, len);
        str[len] = '\\0';
    } else {
        str[0] = '\\0';
    }
}

bool isValidUsername(const char* username) {
    if (username == NULL || strlen(username) < 3 || strlen(username) > 20) {
        return false;
    }
    
    for (int i = 0; username[i]; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return false;
        }
    }
    return true;
}

bool isValidResourceName(const char* resourceName) {
    if (resourceName == NULL || strlen(resourceName) < 1 || strlen(resourceName) > 50) {
        return false;
    }
    
    for (int i = 0; resourceName[i]; i++) {
        if (!isalnum((unsigned char)resourceName[i]) && resourceName[i] != '_') {
            return false;
        }
    }
    return true;
}

char* accessResource(const char* username, const char* resourceName, char* result, size_t resultSize) {
    initializeResources();
    
    if (result == NULL || resultSize == 0) {
        return NULL;
    }
    
    // Input validation
    if (username == NULL || resourceName == NULL) {
        snprintf(result, resultSize, "Error: Invalid input - null values not allowed");
        return result;
    }
    
    // Create copies for sanitization
    char cleanUsername[MAX_USERNAME_LEN];
    char cleanResourceName[MAX_RESOURCE_LEN];
    
    strncpy(cleanUsername, username, MAX_USERNAME_LEN - 1);
    cleanUsername[MAX_USERNAME_LEN - 1] = '\\0';
    trimString(cleanUsername);
    
    strncpy(cleanResourceName, resourceName, MAX_RESOURCE_LEN - 1);
    cleanResourceName[MAX_RESOURCE_LEN - 1] = '\\0';
    trimString(cleanResourceName);
    
    // Validate username format
    if (!isValidUsername(cleanUsername)) {
        snprintf(result, resultSize, "Error: Invalid username format");
        return result;
    }
    
    // Validate resource name format
    if (!isValidResourceName(cleanResourceName)) {
        snprintf(result, resultSize, "Error: Invalid resource name format");
        return result;
    }
    
    // Find user
    int userIndex = -1;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, cleanUsername) == 0) {
            userIndex = i;
            break;
        }
    }
    
    if (userIndex == -1) {
        snprintf(result, resultSize, "Error: User not found");
        return result;
    }
    
    // Find resource
    for (int i = 0; i < users[userIndex].resourceCount; i++) {
        if (strcmp(users[userIndex].resources[i].name, cleanResourceName) == 0) {
            snprintf(result, resultSize, "Success: %s", users[userIndex].resources[i].content);
            return result;
        }
    }
    
    snprintf(result, resultSize, "Error: Access denied - Resource not found or unauthorized");
    return result;
}

int main() {
    char result[512];
    
    printf("=== Secure Resource Access Control System ===\\n\\n");
    
    // Test Case 1: Valid access
    printf("Test 1 - Valid Access:\\n");
    accessResource("user1", "document1", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    // Test Case 2: Valid access different user
    printf("Test 2 - Valid Access (User2):\\n");
    accessResource("user2", "file3", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    // Test Case 3: Unauthorized access attempt
    printf("Test 3 - Unauthorized Access:\\n");
    accessResource("user1", "file3", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    // Test Case 4: Path traversal attempt
    printf("Test 4 - Path Traversal Attack Attempt:\\n");
    accessResource("user1", "../admin/config", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    // Test Case 5: Null input handling
    printf("Test 5 - Null Input:\\n");
    accessResource(NULL, "document1", result, sizeof(result));
    printf("%s\\n\\n", result);
    
    return 0;
}
