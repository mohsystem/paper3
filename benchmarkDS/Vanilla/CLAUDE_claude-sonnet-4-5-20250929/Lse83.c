
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* processPayload(const char* payload) {
    static char result[256];
    char temp[1024];
    strcpy(temp, payload);
    
    // Simple parser to find "name" field
    char* nameStart = strstr(temp, "\\"name\\"");
    if (nameStart == NULL) {
        if (strstr(temp, "{") != NULL && strstr(temp, "}") != NULL) {
            strcpy(result, "Field 'name' not found");
        } else {
            strcpy(result, "Invalid JSON payload");
        }
        return result;
    }
    
    // Find the value after "name":
    char* colonPos = strchr(nameStart, ':');
    if (colonPos == NULL) {
        strcpy(result, "Invalid JSON payload");
        return result;
    }
    
    // Skip whitespace and opening quote
    colonPos++;
    while (*colonPos == ' ' || *colonPos == '\\t') colonPos++;
    
    if (*colonPos == '\\"') {
        colonPos++;
        char* endQuote = strchr(colonPos, '\\"');
        if (endQuote != NULL) {
            int len = endQuote - colonPos;
            strncpy(result, colonPos, len);
            result[len] = '\\0';
            return result;
        }
    }
    
    strcpy(result, "Invalid JSON payload");
    return result;
}

int main() {
    // Test cases
    printf("Test Case 1: %s\\n", processPayload("{\\"name\\":\\"John Doe\\"}"));
    printf("Test Case 2: %s\\n", processPayload("{\\"name\\":\\"Alice\\",\\"age\\":30}"));
    printf("Test Case 3: %s\\n", processPayload("{\\"age\\":25}"));
    printf("Test Case 4: %s\\n", processPayload("{\\"name\\":\\"\\"}"));
    printf("Test Case 5: %s\\n", processPayload("invalid json"));
    
    return 0;
}
