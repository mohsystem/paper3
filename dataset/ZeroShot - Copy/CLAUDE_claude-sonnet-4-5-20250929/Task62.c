
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

cJSON* parseAndGetRoot(const char* jsonString) {
    if (jsonString == NULL || strlen(jsonString) == 0) {
        return NULL;
    }
    
    cJSON* rootElement = cJSON_Parse(jsonString);
    
    if (rootElement == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Invalid JSON syntax: %s\\n", error_ptr);
        }
        return NULL;
    }
    
    return rootElement;
}

void printJSON(cJSON* json) {
    if (json == NULL) {
        printf("NULL\\n");
        return;
    }
    
    char* jsonString = cJSON_Print(json);
    if (jsonString != NULL) {
        printf("%s\\n", jsonString);
        free(jsonString);
    }
}

int main() {
    // Test case 1: Simple JSON object
    const char* test1 = "{\\"name\\": \\"John\\", \\"age\\": 30}";
    printf("Test 1 Root: ");
    cJSON* root1 = parseAndGetRoot(test1);
    printJSON(root1);
    cJSON_Delete(root1);
    
    // Test case 2: JSON array
    const char* test2 = "[1, 2, 3, 4, 5]";
    printf("Test 2 Root: ");
    cJSON* root2 = parseAndGetRoot(test2);
    printJSON(root2);
    cJSON_Delete(root2);
    
    // Test case 3: Nested JSON object
    const char* test3 = "{\\"user\\": {\\"name\\": \\"Alice\\", \\"address\\": {\\"city\\": \\"NYC\\"}}}";
    printf("Test 3 Root: ");
    cJSON* root3 = parseAndGetRoot(test3);
    printJSON(root3);
    cJSON_Delete(root3);
    
    // Test case 4: Empty JSON object
    const char* test4 = "{}";
    printf("Test 4 Root: ");
    cJSON* root4 = parseAndGetRoot(test4);
    printJSON(root4);
    cJSON_Delete(root4);
    
    // Test case 5: Invalid JSON (should handle gracefully)
    const char* test5 = "{invalid json}";
    printf("Test 5 Root: ");
    cJSON* root5 = parseAndGetRoot(test5);
    printJSON(root5);
    cJSON_Delete(root5);
    
    return 0;
}
