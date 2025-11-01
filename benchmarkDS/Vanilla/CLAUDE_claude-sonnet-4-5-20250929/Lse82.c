
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Simple JSON parser for name extraction
char* getNameFromPayload(const char* payload) {
    static char result[256];
    
    const char* namePos = strstr(payload, "\\"name\\"");
    if (namePos == NULL) {
        strcpy(result, "Name not found");
        return result;
    }
    
    const char* colonPos = strchr(namePos, ':');
    if (colonPos == NULL) {
        strcpy(result, "Invalid payload");
        return result;
    }
    
    const char* startQuote = strchr(colonPos, '"');
    if (startQuote == NULL) {
        strcpy(result, "Invalid payload");
        return result;
    }
    startQuote++;
    
    const char* endQuote = strchr(startQuote, '"');
    if (endQuote == NULL) {
        strcpy(result, "Invalid payload");
        return result;
    }
    
    int len = endQuote - startQuote;
    strncpy(result, startQuote, len);
    result[len] = '\\0';
    
    return result;
}

// Note: Full web server implementation in C requires external libraries
int main() {
    printf("Test Cases:\\n");
    printf("1. %s\\n", getNameFromPayload("{\\"name\\":\\"John\\"}"));
    printf("2. %s\\n", getNameFromPayload("{\\"name\\":\\"Alice\\",\\"age\\":30}"));
    printf("3. %s\\n", getNameFromPayload("{\\"age\\":25}"));
    printf("4. %s\\n", getNameFromPayload("invalid json"));
    printf("5. %s\\n", getNameFromPayload("{\\"name\\":\\"\\"}"));
    
    printf("\\nNote: Full web server requires external libraries like libmicrohttpd\\n");
    
    return 0;
}
