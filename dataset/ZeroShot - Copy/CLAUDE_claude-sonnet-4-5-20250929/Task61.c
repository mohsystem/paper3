
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

char* parseAndGetRootElement(const char* xmlString) {
    if (xmlString == NULL) {
        char* error = (char*)malloc(50);
        strcpy(error, "Error: NULL input");
        return error;
    }
    
    const char* start = strchr(xmlString, '<');
    if (start == NULL) {
        char* error = (char*)malloc(50);
        strcpy(error, "Error: No opening tag found");
        return error;
    }
    
    // Skip XML declaration if present
    if (strncmp(start, "<?xml", 5) == 0) {
        start = strstr(start, "?>");
        if (start == NULL) {
            char* error = (char*)malloc(50);
            strcpy(error, "Error: Invalid XML declaration");
            return error;
        }
        start = strchr(start + 2, '<');
    }
    
    if (start == NULL) {
        char* error = (char*)malloc(50);
        strcpy(error, "Error: No root element found");
        return error;
    }
    
    start++; // Skip the '<'
    
    // Find the end of the tag name
    const char* end = start;
    while (*end && *end != '>' && *end != ' ' && *end != '/' && 
           *end != '\\t' && *end != '\\n') {
        end++;
    }
    
    size_t length = end - start;
    char* rootElement = (char*)malloc(length + 1);
    strncpy(rootElement, start, length);
    rootElement[length] = '\\0';
    
    return rootElement;
}

int main() {
    char* result;
    
    // Test case 1: Simple XML
    const char* test1 = "<?xml version=\\"1.0\\"?><root><child>value</child></root>";
    result = parseAndGetRootElement(test1);
    printf("Test 1 - Root element: %s\\n", result);
    free(result);
    
    // Test case 2: XML with attributes
    const char* test2 = "<?xml version=\\"1.0\\"?><books><book id=\\"1\\">Java</book></books>";
    result = parseAndGetRootElement(test2);
    printf("Test 2 - Root element: %s\\n", result);
    free(result);
    
    // Test case 3: Complex XML
    const char* test3 = "<?xml version=\\"1.0\\"?><company><employee><name>John</name></employee></company>";
    result = parseAndGetRootElement(test3);
    printf("Test 3 - Root element: %s\\n", result);
    free(result);
    
    // Test case 4: XML with namespaces
    const char* test4 = "<?xml version=\\"1.0\\"?><ns:document xmlns:ns=\\"http://example.com\\"><ns:data>test</ns:data></ns:document>";
    result = parseAndGetRootElement(test4);
    printf("Test 4 - Root element: %s\\n", result);
    free(result);
    
    // Test case 5: Single element XML
    const char* test5 = "<?xml version=\\"1.0\\"?><message/>";
    result = parseAndGetRootElement(test5);
    printf("Test 5 - Root element: %s\\n", result);
    free(result);
    
    return 0;
}
