
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: C does not have native JNDI support as it's a Java technology.
// This implementation simulates JNDI functionality using a simple registry structure.

#define MAX_RESOURCES 100
#define MAX_NAME_LENGTH 256

typedef enum {
    TYPE_STRING,
    TYPE_INTEGER,
    TYPE_DOUBLE,
    TYPE_UNKNOWN
} ResourceType;

typedef struct {
    char name[MAX_NAME_LENGTH];
    void* data;
    ResourceType type;
    int isActive;
} JNDIResource;

typedef struct {
    JNDIResource resources[MAX_RESOURCES];
    int count;
} JNDIRegistry;

JNDIRegistry registry = {.count = 0};

int bindResource(const char* jndiName, void* resource, ResourceType type) {
    if (registry.count >= MAX_RESOURCES) {
        fprintf(stderr, "Registry is full\\n");
        return 0;
    }
    
    // Check if already exists and update
    for (int i = 0; i < registry.count; i++) {
        if (strcmp(registry.resources[i].name, jndiName) == 0 && 
            registry.resources[i].isActive) {
            if (registry.resources[i].data != NULL) {
                free(registry.resources[i].data);
            }
            registry.resources[i].data = resource;
            registry.resources[i].type = type;
            printf("Successfully rebound: %s\\n", jndiName);
            return 1;
        }
    }
    
    // Add new resource
    strncpy(registry.resources[registry.count].name, jndiName, MAX_NAME_LENGTH - 1);
    registry.resources[registry.count].data = resource;
    registry.resources[registry.count].type = type;
    registry.resources[registry.count].isActive = 1;
    registry.count++;
    
    printf("Successfully bound: %s\\n", jndiName);
    return 1;
}

void* performJNDILookup(const char* jndiName, ResourceType* type) {
    for (int i = 0; i < registry.count; i++) {
        if (strcmp(registry.resources[i].name, jndiName) == 0 && 
            registry.resources[i].isActive) {
            if (type != NULL) {
                *type = registry.resources[i].type;
            }
            return registry.resources[i].data;
        }
    }
    
    fprintf(stderr, "JNDI Lookup failed for: %s\\n", jndiName);
    fprintf(stderr, "Error: Resource not found\\n");
    return NULL;
}

void listBindings(const char* contextName) {
    printf("Bindings in context: %s\\n", contextName);
    int contextLen = strlen(contextName);
    
    for (int i = 0; i < registry.count; i++) {
        if (registry.resources[i].isActive && 
            strncmp(registry.resources[i].name, contextName, contextLen) == 0) {
            const char* typeName;
            switch (registry.resources[i].type) {
                case TYPE_STRING: typeName = "String"; break;
                case TYPE_INTEGER: typeName = "Integer"; break;
                case TYPE_DOUBLE: typeName = "Double"; break;
                default: typeName = "Unknown"; break;
            }
            printf("  Name: %s, Type: %s\\n", registry.resources[i].name, typeName);
        }
    }
}

int unbindResource(const char* jndiName) {
    for (int i = 0; i < registry.count; i++) {
        if (strcmp(registry.resources[i].name, jndiName) == 0 && 
            registry.resources[i].isActive) {
            if (registry.resources[i].data != NULL) {
                free(registry.resources[i].data);
                registry.resources[i].data = NULL;
            }
            registry.resources[i].isActive = 0;
            printf("Successfully unbound: %s\\n", jndiName);
            return 1;
        }
    }
    
    fprintf(stderr, "Resource not found: %s\\n", jndiName);
    return 0;
}

int main() {
    printf("=== JNDI Lookup Simulation (C) ===\\n\\n");
    
    // Test Case 1: Bind and lookup a String
    printf("Test Case 1: String Resource\\n");
    char* testString = malloc(strlen("Hello JNDI World") + 1);
    strcpy(testString, "Hello JNDI World");
    bindResource("test/string", testString, TYPE_STRING);
    ResourceType type1;
    char* result1 = (char*)performJNDILookup("test/string", &type1);
    if (result1) {
        printf("Retrieved: %s\\n", result1);
    }
    printf("\\n");
    
    // Test Case 2: Bind and lookup an Integer
    printf("Test Case 2: Integer Resource\\n");
    int* testInteger = malloc(sizeof(int));
    *testInteger = 12345;
    bindResource("test/integer", testInteger, TYPE_INTEGER);
    ResourceType type2;
    int* result2 = (int*)performJNDILookup("test/integer", &type2);
    if (result2) {
        printf("Retrieved: %d\\n", *result2);
    }
    printf("\\n");
    
    // Test Case 3: Bind and lookup a Double
    printf("Test Case 3: Double Resource\\n");
    double* testDouble = malloc(sizeof(double));
    *testDouble = 3.14159;
    bindResource("test/double", testDouble, TYPE_DOUBLE);
    ResourceType type3;
    double* result3 = (double*)performJNDILookup("test/double", &type3);
    if (result3) {
        printf("Retrieved: %.5f\\n", *result3);
    }
    printf("\\n");
    
    // Test Case 4: List all bindings
    printf("Test Case 4: List Bindings\\n");
    listBindings("test");
    printf("\\n");
    
    // Test Case 5: Lookup non-existent resource
    printf("Test Case 5: Non-existent Resource\\n");
    ResourceType type5;
    void* result5 = performJNDILookup("test/nonexistent", &type5);
    printf("Retrieved: %s\\n", result5 ? "Found" : "NULL");
    
    return 0;
}
