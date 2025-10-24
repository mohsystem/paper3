
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// Note: For production use, consider using libxml2 with proper security settings
// This implementation provides basic XML parsing with security considerations

char* parseAndGetRootElement(const char* xmlString) {
    static char result[256];
    
    if (xmlString == NULL || strlen(xmlString) == 0) {
        strcpy(result, "Error: Empty or null XML string");
        return result;
    }
    
    // Find the start of the root element
    const char* rootStart = strchr(xmlString, '<');
    if (rootStart == NULL) {
        strcpy(result, "Error: No XML element found");
        return result;
    }
    
    // Skip XML declaration if present
    if (strncmp(rootStart, "<?xml", 5) == 0) {
        rootStart = strstr(rootStart, "?>");
        if (rootStart == NULL) {
            strcpy(result, "Error: Malformed XML declaration");
            return result;
        }
        rootStart = strchr(rootStart + 2, '<');
    }
    
    if (rootStart == NULL) {
        strcpy(result, "Error: No root element found");
        return result;
    }
    
    // Extract root element name
    const char* nameStart = rootStart + 1;
    const char* nameEnd = nameStart;
    
    while (*nameEnd != '\\0' && 
           *nameEnd != '>' && 
           *nameEnd != ' ' && 
           *nameEnd != '\\t' &&
           *nameEnd != '\\n' &&
           *nameEnd != '\\r') {
        nameEnd++;
    }
    
    size_t nameLen = nameEnd - nameStart;
    if (nameLen > 0 && nameLen < sizeof(result) - 1) {
        strncpy(result, nameStart, nameLen);
        result[nameLen] = '\\0';
        return result;
    }
    
    strcpy(result, "Error: Could not extract root element name");
    return result;
}

int main() {
    // Test case 1: Simple XML
    const char* test1 = "<?xml version=\\"1.0\\"?><root><child>data</child></root>";
    printf("Test 1 - Root element: %s\\n", parseAndGetRootElement(test1));
    
    // Test case 2: Complex XML
    const char* test2 = "<?xml version=\\"1.0\\" encoding=\\"UTF-8\\"?><books><book id=\\"1\\"><title>Java</title></book></books>";
    printf("Test 2 - Root element: %s\\n", parseAndGetRootElement(test2));
    
    // Test case 3: Different root element
    const char* test3 = "<catalog><item>Product</item></catalog>";
    printf("Test 3 - Root element: %s\\n", parseAndGetRootElement(test3));
    
    // Test case 4: Malformed XML
    const char* test4 = "<root><unclosed>";
    printf("Test 4 - Root element: %s\\n", parseAndGetRootElement(test4));
    
    // Test case 5: Empty string
    const char* test5 = "";
    printf("Test 5 - Root element: %s\\n", parseAndGetRootElement(test5));
    
    return 0;
}
