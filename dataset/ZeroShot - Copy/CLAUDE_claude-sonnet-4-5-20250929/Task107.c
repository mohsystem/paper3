
// C does not have native JNDI support as it's a Java-specific API\n// This is a basic simulation using string validation\n\n#include <stdio.h>\n#include <string.h>\n#include <stdlib.h>\n#include <ctype.h>\n\n#define MAX_NAME_LENGTH 256\n#define MAX_REGISTRY_SIZE 100\n\ntypedef struct {\n    char name[MAX_NAME_LENGTH];\n    void* resource;\n    int is_used;\n} RegistryEntry;\n\ntypedef struct {\n    RegistryEntry entries[MAX_REGISTRY_SIZE];\n    int count;\n} LocalRegistry;\n\nLocalRegistry globalRegistry = {0};\n\n// Check if string starts with a prefix\nint starts_with(const char* str, const char* prefix) {\n    if (!str || !prefix) return 0;\n    size_t len_prefix = strlen(prefix);\n    size_t len_str = strlen(str);\n    if (len_str < len_prefix) return 0;\n    return strncmp(str, prefix, len_prefix) == 0;\n}\n\n// Check if string is empty or whitespace only\nint is_empty_or_whitespace(const char* str) {\n    if (!str || strlen(str) == 0) return 1;\n    while (*str) {\n        if (!isspace((unsigned char)*str)) return 0;\n        str++;\n    }\n    return 1;\n}\n\n// Check if JNDI name uses dangerous protocol\nint is_dangerous_protocol(const char* jndi_name) {\n    if (!jndi_name) return 1;\n    \n    if (starts_with(jndi_name, "ldap://") ||\n        starts_with(jndi_name, "LDAP://") ||\n        starts_with(jndi_name, "rmi://") ||\n        starts_with(jndi_name, "RMI://") ||\n        starts_with(jndi_name, "dns://") ||\n        starts_with(jndi_name, "DNS://") ||\n        starts_with(jndi_name, "iiop://") ||\n        starts_with(jndi_name, "IIOP://")) {\n        return 1;\n    }\n    return 0;\n}\n\n// Secure JNDI lookup simulation\nvoid* perform_jndi_lookup(const char* jndi_name) {\n    // Input validation\n    if (is_empty_or_whitespace(jndi_name)) {\n        fprintf(stderr, "Error: JNDI name cannot be null or empty\\n");\n        return NULL;\n    }\n    \n    // Whitelist validation - block remote protocols\n    if (is_dangerous_protocol(jndi_name)) {\n        fprintf(stderr, "Security Error: Remote JNDI lookups are not allowed for security reasons\\n");\n        return NULL;\n    }\n    \n    // Only allow java: namespace\n    if (!starts_with(jndi_name, "java:")) {\n        fprintf(stderr, "Security Error: Only java: namespace is allowed\\n");\n        return NULL;\n    }\n    \n    // Search in local registry\n    for (int i = 0; i < globalRegistry.count; i++) {\n        if (globalRegistry.entries[i].is_used &&\n            strcmp(globalRegistry.entries[i].name, jndi_name) == 0) {\n            printf("Successfully retrieved resource: %s\\n", jndi_name);\n            return globalRegistry.entries[i].resource;\n        }\n    }\n    \n    printf("Resource not found: %s\\n", jndi_name);\n    return NULL;\n}\n\n// Bind resource to registry\nint bind_resource(const char* jndi_name, void* resource) {\n    if (is_empty_or_whitespace(jndi_name) || !starts_with(jndi_name, "java:")) {\n        fprintf(stderr, "Security Error: Can only bind to java: namespace\\n");\n        return -1;\n    }\n    \n    if (globalRegistry.count >= MAX_REGISTRY_SIZE) {\n        fprintf(stderr, "Error: Registry is full\\n");\n        return -1;\n    }\n    \n    strncpy(globalRegistry.entries[globalRegistry.count].name, jndi_name, MAX_NAME_LENGTH - 1);\n    globalRegistry.entries[globalRegistry.count].name[MAX_NAME_LENGTH - 1] = '\\0';
    globalRegistry.entries[globalRegistry.count].resource = resource;
    globalRegistry.entries[globalRegistry.count].is_used = 1;
    globalRegistry.count++;
    
    printf("Successfully bound resource: %s\\n", jndi_name);
    return 0;
}

int main() {
    printf("=== JNDI Lookup Security Demo (C Simulation) ===\\n\\n");
    
    // Bind a test resource
    char test_data[] = "TestDatabase";
    bind_resource("java:comp/env/example", test_data);
    printf("\\n");
    
    // Test Case 1: Valid local lookup
    printf("Test Case 1: Valid local java: namespace lookup\\n");
    void* result1 = perform_jndi_lookup("java:comp/env/example");
    printf("Result: %s\\n\\n", result1 ? "Found" : "Not found");
    
    // Test Case 2: Attempting LDAP lookup (should be blocked)
    printf("Test Case 2: Blocked LDAP lookup attempt\\n");
    void* result2 = perform_jndi_lookup("ldap://malicious.com/exploit");
    printf("\\n");
    
    // Test Case 3: Attempting RMI lookup (should be blocked)
    printf("Test Case 3: Blocked RMI lookup attempt\\n");
    void* result3 = perform_jndi_lookup("rmi://malicious.com/exploit");
    printf("\\n");
    
    // Test Case 4: NULL JNDI name
    printf("Test Case 4: NULL JNDI name validation\\n");
    void* result4 = perform_jndi_lookup(NULL);
    printf("\\n");
    
    // Test Case 5: Empty JNDI name
    printf("Test Case 5: Empty JNDI name validation\\n");
    void* result5 = perform_jndi_lookup("");
    printf("\\n");
    
    printf("=== Security Summary ===\\n");
    printf("✓ Remote lookups (LDAP, RMI, DNS, IIOP) are blocked\\n");
    printf("✓ Only local java: namespace is allowed\\n");
    printf("✓ Input validation prevents injection attacks\\n");
    printf("✓ Proper error handling\\n");
    
    return 0;
}
