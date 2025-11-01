
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// Validate username (alphanumeric, underscore, hyphen only)
bool isValidUsername(const char* username) {
    if (username == NULL || strlen(username) == 0) {
        return false;
    }
    
    for (int i = 0; username[i] != '\\0'; i++) {
        if (!isalnum(username[i]) && username[i] != '_' && username[i] != '-') {
            return false;
        }
    }
    return true;
}

void getUserInfo(const char* username, char* result, size_t resultSize) {
    result[0] = '\\0';
    
    if (!isValidUsername(username)) {
        snprintf(result, resultSize, "Error: Invalid username format");
        return;
    }
    
    // Directly read the file without shell execution
    FILE* file = fopen("/etc/passwd", "r");
    if (file == NULL) {
        snprintf(result, resultSize, "Error: Cannot open /etc/passwd");
        return;
    }
    
    char line[512];
    char searchPrefix[128];
    snprintf(searchPrefix, sizeof(searchPrefix), "%s:", username);
    size_t prefixLen = strlen(searchPrefix);
    
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strncmp(line, searchPrefix, prefixLen) == 0) {
            // Remove newline
            line[strcspn(line, "\\n")] = '\\0';
            strncpy(result, line, resultSize - 1);
            result[resultSize - 1] = '\\0';
            fclose(file);
            return;
        }
    }
    
    fclose(file);
}

int main() {
    const char* testUsers[] = {"root", "daemon", "nobody", "testuser", "admin"};
    int numTests = sizeof(testUsers) / sizeof(testUsers[0]);
    char result[512];
    
    printf("=== User Information Lookup (Secure) ===\\n\\n");
    
    for (int i = 0; i < numTests; i++) {
        printf("Searching for user: %s\\n", testUsers[i]);
        getUserInfo(testUsers[i], result, sizeof(result));
        printf("%s\\n", strlen(result) > 0 ? result : "User not found");
        printf("---\\n");
    }
    
    return 0;
}
