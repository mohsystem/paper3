
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PAYLOAD_SIZE 10
#define MAX_STRING_SIZE 256

typedef struct {
    char key[MAX_STRING_SIZE];
    char value[MAX_STRING_SIZE];
} KeyValue;

typedef struct {
    KeyValue items[MAX_PAYLOAD_SIZE];
    int size;
} Payload;

void sanitizeInput(const char* input, char* output, int maxSize) {
    int j = 0;
    for (int i = 0; input[i] != '\\0' && j < maxSize - 10; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&output[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&output[j], "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(&output[j], "&#x2F;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

char* processPayload(Payload* payload, char* result, int maxSize) {
    if (payload == NULL) {
        strncpy(result, "Error: Invalid payload", maxSize);
        return result;
    }
    
    for (int i = 0; i < payload->size; i++) {
        if (strcmp(payload->items[i].key, "name") == 0) {
            char sanitized[MAX_STRING_SIZE * 6];
            sanitizeInput(payload->items[i].value, sanitized, sizeof(sanitized));
            snprintf(result, maxSize, "Name: %s", sanitized);
            return result;
        }
    }
    
    strncpy(result, "Error: Field 'name' not found", maxSize);
    return result;
}

int main() {
    char result[MAX_STRING_SIZE * 6];
    
    // Test Case 1
    printf("Test Case 1:\\n");
    Payload payload1 = {.size = 1};
    strcpy(payload1.items[0].key, "name");
    strcpy(payload1.items[0].value, "John Doe");
    printf("%s\\n\\n", processPayload(&payload1, result, sizeof(result)));
    
    // Test Case 2
    printf("Test Case 2:\\n");
    Payload payload2 = {.size = 1};
    strcpy(payload2.items[0].key, "email");
    strcpy(payload2.items[0].value, "test@example.com");
    printf("%s\\n\\n", processPayload(&payload2, result, sizeof(result)));
    
    // Test Case 3
    printf("Test Case 3:\\n");
    Payload payload3 = {.size = 1};
    strcpy(payload3.items[0].key, "name");
    strcpy(payload3.items[0].value, "<script>alert('XSS')</script>");
    printf("%s\\n\\n", processPayload(&payload3, result, sizeof(result)));
    
    // Test Case 4
    printf("Test Case 4:\\n");
    Payload payload4 = {.size = 1};
    strcpy(payload4.items[0].key, "name");
    strcpy(payload4.items[0].value, "Alice & Bob");
    printf("%s\\n\\n", processPayload(&payload4, result, sizeof(result)));
    
    // Test Case 5
    printf("Test Case 5:\\n");
    printf("%s\\n", processPayload(NULL, result, sizeof(result)));
    
    return 0;
}
