
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LENGTH 256
#define MAX_BUFFER_SIZE 1024

typedef struct {
    char name[MAX_NAME_LENGTH];
    int value;
} SafeData;

// Serialize SafeData to string format
char* serializeObject(const SafeData* data) {
    char* buffer = (char*)malloc(MAX_BUFFER_SIZE);
    if (!buffer) return NULL;
    
    snprintf(buffer, MAX_BUFFER_SIZE, "SAFEDATA|%zu|%s|%d", 
             strlen(data->name), data->name, data->value);
    return buffer;
}

// Securely deserialize with validation
int deserializeSecurely(const char* serialized, SafeData* data) {
    if (!serialized || !data) {
        fprintf(stderr, "Invalid parameters\\n");
        return -1;
    }
    
    char type[32];
    size_t nameLength;
    
    // Parse: SAFEDATA|length|name|value
    int result = sscanf(serialized, "%31[^|]|%zu|", type, &nameLength);
    
    if (result != 2 || strcmp(type, "SAFEDATA") != 0) {
        fprintf(stderr, "Unauthorized deserialization attempt: %s\\n", type);
        return -1;
    }
    
    if (nameLength >= MAX_NAME_LENGTH) {
        fprintf(stderr, "Name length exceeds maximum\\n");
        return -1;
    }
    
    // Find the start of name
    const char* nameStart = strchr(serialized, '|');
    if (!nameStart) return -1;
    nameStart = strchr(nameStart + 1, '|');
    if (!nameStart) return -1;
    nameStart++;
    
    // Copy name
    strncpy(data->name, nameStart, nameLength);
    data->name[nameLength] = '\\0';
    
    // Parse value
    const char* valueStart = nameStart + nameLength + 1;
    if (sscanf(valueStart, "%d", &data->value) != 1) {
        fprintf(stderr, "Invalid value format\\n");
        return -1;
    }
    
    return 0;
}

// JSON-like serialization
char* serializeToJSON(const SafeData* data) {
    char* buffer = (char*)malloc(MAX_BUFFER_SIZE);
    if (!buffer) return NULL;
    
    snprintf(buffer, MAX_BUFFER_SIZE, "{\\"name\\":\\"%s\\",\\"value\\":%d}", 
             data->name, data->value);
    return buffer;
}

// JSON-like deserialization (simplified)
int deserializeFromJSON(const char* json, SafeData* data) {
    if (!json || !data) return -1;
    
    const char* nameStart = strstr(json, "\\"name\\":\\"");
    if (!nameStart) return -1;
    nameStart += 8;
    
    const char* nameEnd = strchr(nameStart, '"');
    if (!nameEnd) return -1;
    
    size_t nameLen = nameEnd - nameStart;
    if (nameLen >= MAX_NAME_LENGTH) return -1;
    
    strncpy(data->name, nameStart, nameLen);
    data->name[nameLen] = '\\0';
    
    const char* valueStart = strstr(json, "\\"value\\":");
    if (!valueStart) return -1;
    valueStart += 8;
    
    if (sscanf(valueStart, "%d", &data->value) != 1) return -1;
    
    return 0;
}

void printSafeData(const char* label, const SafeData* data) {
    printf("%s: SafeData{name='%s', value=%d}\\n", label, data->name, data->value);
}

int main() {
    printf("=== Secure Deserialization Tests ===\\n\\n");
    
    // Test Case 1: Deserialize SafeData object
    printf("Test 1: Deserialize SafeData object\\n");
    SafeData original1 = {"TestData", 42};
    char* serialized1 = serializeObject(&original1);
    if (serialized1) {
        SafeData deserialized1;
        if (deserializeSecurely(serialized1, &deserialized1) == 0) {
            printSafeData("Original", &original1);
            printSafeData("Deserialized", &deserialized1);
            printf("Success!\\n\\n");
        }
        free(serialized1);
    }
    
    // Test Case 2: Deserialize with different values
    printf("Test 2: Deserialize with different values\\n");
    SafeData original2 = {"SecureData", 12345};
    char* serialized2 = serializeObject(&original2);
    if (serialized2) {
        SafeData deserialized2;
        if (deserializeSecurely(serialized2, &deserialized2) == 0) {
            printSafeData("Original", &original2);
            printSafeData("Deserialized", &deserialized2);
            printf("Success!\\n\\n");
        }
        free(serialized2);
    }
    
    // Test Case 3: JSON-based serialization
    printf("Test 3: JSON-based serialization\\n");
    SafeData original3 = {"JSONData", 999};
    char* json = serializeToJSON(&original3);
    if (json) {
        SafeData deserialized3;
        if (deserializeFromJSON(json, &deserialized3) == 0) {
            printSafeData("Original", &original3);
            printf("JSON: %s\\n", json);
            printSafeData("Deserialized", &deserialized3);
            printf("Success!\\n\\n");
        }
        free(json);
    }
    
    // Test Case 4: Empty string handling
    printf("Test 4: Empty string handling\\n");
    SafeData original4 = {"", 0};
    char* serialized4 = serializeObject(&original4);
    if (serialized4) {
        SafeData deserialized4;
        if (deserializeSecurely(serialized4, &deserialized4) == 0) {
            printSafeData("Original", &original4);
            printSafeData("Deserialized", &deserialized4);
            printf("Success!\\n\\n");
        }
        free(serialized4);
    }
    
    // Test Case 5: Invalid format detection
    printf("Test 5: Invalid format detection (should fail)\\n");
    const char* invalid = "INVALID|DATA|FORMAT";
    SafeData deserialized5;
    if (deserializeSecurely(invalid, &deserialized5) != 0) {
        printf("Expected error caught - invalid format rejected\\n");
        printf("Success!\\n\\n");
    }
    
    return 0;
}
