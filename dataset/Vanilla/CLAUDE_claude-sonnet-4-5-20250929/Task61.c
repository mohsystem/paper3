
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* getRootElement(const char* xmlString) {
    static char result[256];
    const char* start = strchr(xmlString, '<');
    
    if (start == NULL) {
        strcpy(result, "Error: No XML tags found");
        return result;
    }
    
    // Skip XML declaration if present
    if (strncmp(start, "<?xml", 5) == 0) {
        start = strstr(start, "?>");
        if (start != NULL) {
            start = strchr(start + 2, '<');
        }
    }
    
    if (start == NULL) {
        strcpy(result, "Error: No root element found");
        return result;
    }
    
    start++; // Skip the '<' character
    int i = 0;
    
    // Extract tag name until space, '>', or '/'
    while (start[i] != '\\0' && start[i] != '>' && start[i] != ' ' && start[i] != '/') {
        result[i] = start[i];
        i++;
        if (i >= 255) break;
    }
    result[i] = '\\0';
    
    return result;
}

int main() {
    // Test case 1: Simple XML
    const char* xml1 = "<?xml version=\\"1.0\\"?><root><child>value</child></root>";
    printf("Test 1 - Root element: %s\\n", getRootElement(xml1));
    
    // Test case 2: XML with attributes
    const char* xml2 = "<?xml version=\\"1.0\\"?><book id=\\"1\\"><title>Java Programming</title></book>";
    printf("Test 2 - Root element: %s\\n", getRootElement(xml2));
    
    // Test case 3: XML with nested elements
    const char* xml3 = "<?xml version=\\"1.0\\"?><catalog><product><name>Item1</name></product></catalog>";
    printf("Test 3 - Root element: %s\\n", getRootElement(xml3));
    
    // Test case 4: XML with different root
    const char* xml4 = "<?xml version=\\"1.0\\"?><students><student><name>John</name></student></students>";
    printf("Test 4 - Root element: %s\\n", getRootElement(xml4));
    
    // Test case 5: Simple root without declaration
    const char* xml5 = "<data><item>test</item></data>";
    printf("Test 5 - Root element: %s\\n", getRootElement(xml5));
    
    return 0;
}
