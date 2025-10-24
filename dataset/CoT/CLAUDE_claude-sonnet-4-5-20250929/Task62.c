
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

cJSON* parseAndGetRoot(const char* jsonString) {
    if (jsonString == NULL || strlen(jsonString) == 0) {
        fprintf(stderr, "Empty JSON string\\n");
        return NULL;
    }
    
    // Limit input size to prevent resource exhaustion
    if (strlen(jsonString) > 10000000) {
        fprintf(stderr, "JSON string too large\\n");
        return NULL;
    }
    
    cJSON* rootElement = cJSON_Parse(jsonString);
    
    if (rootElement == NULL) {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Invalid JSON format\\n");
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
    
    char* jsonStr = cJSON_Print(json);
    if (jsonStr != NULL) {
        printf("%s\\n", jsonStr);
        free(jsonStr);
    }
}

int main() {
    // Test case 1: Simple JSON object
    const char* test1 = "{\\"name\\":\\"John\\",\\"age\\":30}";
    printf("Test 1: ");
    cJSON* result1 = parseAndGetRoot(test1);
    printJSON(result1);
    cJSON_Delete(result1);
    
    // Test case 2: JSON array
    const char* test2 = "[1,2,3,4,5]";
    printf("Test 2: ");
    cJSON* result2 = parseAndGetRoot(test2);
    printJSON(result2);
    cJSON_Delete(result2);
    
    // Test case 3: Nested JSON object
    const char* test3 = "{\\"user\\":{\\"name\\":\\"Alice\\",\\"address\\":{\\"city\\":\\"NYC\\"}}}";
    printf("Test 3: ");
    cJSON* result3 = parseAndGetRoot(test3);
    printJSON(result3);
    cJSON_Delete(result3);
    
    // Test case 4: Empty JSON object
    const char* test4 = "{}";
    printf("Test 4: ");
    cJSON* result4 = parseAndGetRoot(test4);
    printJSON(result4);
    cJSON_Delete(result4);
    
    // Test case 5: Invalid JSON
    const char* test5 = "{invalid json}";
    printf("Test 5: ");
    cJSON* result5 = parseAndGetRoot(test5);
    printJSON(result5);
    cJSON_Delete(result5);
    
    return 0;
}
