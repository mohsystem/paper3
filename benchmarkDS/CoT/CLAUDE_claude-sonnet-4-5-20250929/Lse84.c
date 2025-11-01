
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PAYLOAD_SIZE 10000
#define MAX_QUERY_SIZE 20000

// Simplified URL decode function
void urlDecode(const char* src, char* dest, size_t destSize) {
    size_t i = 0, j = 0;
    while (src[i] && j < destSize - 1) {
        if (src[i] == '%' && src[i+1] && src[i+2]) {
            int value;
            if (sscanf(&src[i+1], "%2x", &value) == 1) {
                dest[j++] = (char)value;
                i += 3;
                continue;
            }
        } else if (src[i] == '+') {
            dest[j++] = ' ';
            i++;
            continue;
        }
        dest[j++] = src[i++];
    }
    dest[j] = '\\0';
}

// Extract payload from query string
int extractPayload(const char* query, char* payload, size_t payloadSize) {
    const char* start = strstr(query, "payload=");
    if (!start) return 0;
    
    start += 8; // Skip "payload="
    const char* end = strchr(start, '&');
    size_t len = end ? (size_t)(end - start) : strlen(start);
    
    if (len >= payloadSize) len = payloadSize - 1;
    strncpy(payload, start, len);
    payload[len] = '\\0';
    
    return 1;
}

// Process the payload (simplified YAML check)
const char* processPayload(const char* payload) {
    if (!payload || strlen(payload) == 0) {
        return "Error: payload parameter is empty";
    }
    
    if (strlen(payload) > MAX_PAYLOAD_SIZE) {
        return "Error: payload too large";
    }
    
    // Simple check for "type: Create" or "type:Create"
    const char* typePos = strstr(payload, "type");
    if (typePos) {
        const char* colonPos = strchr(typePos, ':');
        if (colonPos) {
            colonPos++;
            while (*colonPos == ' ' || *colonPos == '\\t') colonPos++;
            if (strncmp(colonPos, "Create", 6) == 0) {
                return "Error: Create type is not allowed";
            }
        }
    }
    
    return "Success: Payload processed";
}

// Main request handler
const char* handleRequest(const char* query) {
    if (!query || !strstr(query, "payload=")) {
        return "Error: payload parameter is missing";
    }
    
    char rawPayload[MAX_QUERY_SIZE];
    char decodedPayload[MAX_QUERY_SIZE];
    
    if (!extractPayload(query, rawPayload, sizeof(rawPayload))) {
        return "Error: payload parameter is missing";
    }
    
    urlDecode(rawPayload, decodedPayload, sizeof(decodedPayload));
    return processPayload(decodedPayload);
}

// Test cases
void runTests() {
    printf("\\nTest Cases:\\n");
    
    // Test 1: Missing payload
    printf("Test 1: %s\\n", handleRequest(""));
    
    // Test 2: Valid payload with type=Create
    printf("Test 2: %s\\n", handleRequest("payload=type:%20Create"));
    
    // Test 3: Valid payload with type=Update
    printf("Test 3: %s\\n", handleRequest("payload=type:%20Update"));
    
    // Test 4: Empty payload
    printf("Test 4: %s\\n", handleRequest("payload="));
    
    // Test 5: Valid payload without type
    printf("Test 5: %s\\n", handleRequest("payload=data:%20value"));
}

int main() {
    printf("Server simulation on port 5000\\n");
    runTests();
    return 0;
}
