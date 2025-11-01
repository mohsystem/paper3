
// Note: C doesn't have native HTTP server or YAML parsing capabilities.
// This is a simplified implementation showing the core logic.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* processPayload(const char* payload) {
    static char result[100];
    
    // Simplified YAML parsing - just checking for "type:" and "Create"
    if (strstr(payload, "type:") != NULL && strstr(payload, "Create") != NULL) {
        strcpy(result, "Error: Type cannot be Create");
    } else {
        strcpy(result, "Success: Payload processed");
    }
    
    return result;
}

char* handleRequest(const char* payload) {
    static char error[100];
    
    if (payload == NULL || strlen(payload) == 0) {
        strcpy(error, "Error: Missing payload parameter");
        return error;
    }
    
    return processPayload(payload);
}

int main() {
    // Test cases
    printf("Test Case 1: Missing payload parameter\\n");
    printf("%s\\n\\n", handleRequest(NULL));
    
    printf("Test Case 2: Valid YAML with type=Update\\n");
    printf("%s\\n\\n", handleRequest("type: Update\\nvalue: 123"));
    
    printf("Test Case 3: Valid YAML with type=Create\\n");
    printf("%s\\n\\n", handleRequest("type: Create\\nvalue: 123"));
    
    printf("Test Case 4: Valid YAML without type field\\n");
    printf("%s\\n\\n", handleRequest("name: test\\nvalue: 456"));
    
    printf("Test Case 5: Empty payload\\n");
    printf("%s\\n", handleRequest(""));
    
    return 0;
}
