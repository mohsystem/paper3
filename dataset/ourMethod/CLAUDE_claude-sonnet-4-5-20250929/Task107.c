
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_NAME_LENGTH 256
#define MAX_VALUE_LENGTH 512
#define MAX_RESOURCES 100

/* Security: Simulated JNDI-like registry with strict validation */
/* No dynamic loading, no deserialization, local registry only */

typedef enum {
    RESOURCE_TYPE_STRING,
    RESOURCE_TYPE_DATASOURCE
} ResourceType;

typedef struct {
    char name[MAX_NAME_LENGTH + 1];
    ResourceType type;
    char value[MAX_VALUE_LENGTH + 1];
    bool in_use;
} Resource;

typedef struct {
    Resource resources[MAX_RESOURCES];
    int count;
} JNDIContext;

/* Security: Validate JNDI name - only alphanumeric, underscore, slash, dot */
bool is_valid_name(const char* name) {
    size_t len;
    size_t i;
    
    if (name == NULL) {
        return false;
    }
    
    len = strlen(name);
    
    /* Input validation: check length bounds */
    if (len == 0 || len > MAX_NAME_LENGTH) {
        return false;
    }
    
    /* Whitelist validation: only safe characters */
    for (i = 0; i < len; i++) {
        char c = name[i];
        if (!isalnum((unsigned char)c) && c != '_' && c != '/' && c != '.') {
            return false;
        }
    }
    
    return true;
}

/* Security: Detect path traversal attempts */
bool contains_path_traversal(const char* name) {
    if (name == NULL) {
        return true;
    }
    
    /* Check for .. pattern */
    if (strstr(name, "..") != NULL) {
        return true;
    }
    
    /* Check for URL schemes that could indicate remote lookups */
    if (strstr(name, "://") != NULL) {
        return true;
    }
    
    return false;
}

/* Initialize JNDI context */
void jndi_init(JNDIContext* ctx) {
    int i;
    if (ctx == NULL) {
        return;
    }
    
    ctx->count = 0;
    
    /* Initialize all resources */
    for (i = 0; i < MAX_RESOURCES; i++) {
        memset(ctx->resources[i].name, 0, sizeof(ctx->resources[i].name));
        memset(ctx->resources[i].value, 0, sizeof(ctx->resources[i].value));
        ctx->resources[i].in_use = false;
        ctx->resources[i].type = RESOURCE_TYPE_STRING;
    }
}

/* Security: Safe bind with validation, no remote operations */
int jndi_bind(JNDIContext* ctx, const char* name, ResourceType type, const char* value) {
    size_t name_len;
    size_t value_len;
    int i;
    
    /* Input validation: NULL checks */
    if (ctx == NULL || name == NULL || value == NULL) {
        fprintf(stderr, "Error: NULL parameter in jndi_bind\\n");
        return -1;
    }
    
    /* Security: Validate name format */
    if (!is_valid_name(name)) {
        fprintf(stderr, "Error: Invalid JNDI name format\\n");
        return -1;
    }
    
    /* Security: Check for path traversal */
    if (contains_path_traversal(name)) {
        fprintf(stderr, "Error: Path traversal detected in JNDI name\\n");
        return -1;
    }
    
    /* Bounds validation */
    name_len = strlen(name);
    value_len = strlen(value);
    
    if (value_len > MAX_VALUE_LENGTH) {
        fprintf(stderr, "Error: Value too long\\n");
        return -1;
    }
    
    /* Check capacity */
    if (ctx->count >= MAX_RESOURCES) {
        fprintf(stderr, "Error: Resource registry full\\n");
        return -1;
    }
    
    /* Find empty slot */
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (!ctx->resources[i].in_use) {
            /* Safe copy with bounds checking */
            strncpy(ctx->resources[i].name, name, MAX_NAME_LENGTH);
            ctx->resources[i].name[MAX_NAME_LENGTH] = '\\0';
            
            strncpy(ctx->resources[i].value, value, MAX_VALUE_LENGTH);
            ctx->resources[i].value[MAX_VALUE_LENGTH] = '\\0';
            
            ctx->resources[i].type = type;
            ctx->resources[i].in_use = true;
            ctx->count++;
            return 0;
        }
    }
    
    return -1;
}

/* Security: Safe lookup with validation */
const Resource* jndi_lookup(JNDIContext* ctx, const char* name) {
    int i;
    
    /* Input validation */
    if (ctx == NULL || name == NULL) {
        fprintf(stderr, "Error: NULL parameter in jndi_lookup\\n");
        return NULL;
    }
    
    /* Security: Validate name */
    if (!is_valid_name(name)) {
        fprintf(stderr, "Error: Invalid JNDI name format\\n");
        return NULL;
    }
    
    /* Security: Check for path traversal */
    if (contains_path_traversal(name)) {
        fprintf(stderr, "Error: Path traversal detected in JNDI name\\n");
        return NULL;
    }
    
    /* Search for resource */
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (ctx->resources[i].in_use && 
            strcmp(ctx->resources[i].name, name) == 0) {
            return &ctx->resources[i];
        }
    }
    
    fprintf(stderr, "Error: Resource not found: %s\\n", name);
    return NULL;
}

/* Cleanup function */
void jndi_cleanup(JNDIContext* ctx) {
    int i;
    if (ctx == NULL) {
        return;
    }
    
    /* Clear sensitive data before cleanup */
    for (i = 0; i < MAX_RESOURCES; i++) {
        if (ctx->resources[i].in_use) {
            memset(ctx->resources[i].value, 0, sizeof(ctx->resources[i].value));
            memset(ctx->resources[i].name, 0, sizeof(ctx->resources[i].name));
            ctx->resources[i].in_use = false;
        }
    }
    ctx->count = 0;
}

int main(void) {
    JNDIContext ctx;
    const Resource* res;
    
    jndi_init(&ctx);
    
    /* Test case 1: Bind and lookup string resource */
    printf("Test 1: String resource lookup\\n");
    if (jndi_bind(&ctx, "app/config/message", RESOURCE_TYPE_STRING, "Hello JNDI") == 0) {
        res = jndi_lookup(&ctx, "app/config/message");
        if (res != NULL) {
            printf("Type: %s, Value: %s\\n", 
                   res->type == RESOURCE_TYPE_STRING ? "String" : "DataSource",
                   res->value);
        }
    }
    
    /* Test case 2: Bind and lookup datasource */
    printf("\\nTest 2: DataSource resource lookup\\n");
    if (jndi_bind(&ctx, "jdbc/MyDataSource", RESOURCE_TYPE_DATASOURCE, 
                  "jdbc:mysql://localhost:3306/testdb") == 0) {
        res = jndi_lookup(&ctx, "jdbc/MyDataSource");
        if (res != NULL) {
            printf("Type: %s, Value: %s\\n",
                   res->type == RESOURCE_TYPE_STRING ? "String" : "DataSource",
                   res->value);
        }
    }
    
    /* Test case 3: Invalid name with path traversal (should fail) */
    printf("\\nTest 3: Invalid name with path traversal\\n");
    res = jndi_lookup(&ctx, "app/config/../../../etc/passwd");
    if (res == NULL) {
        printf("Correctly rejected malicious path\\n");
    }
    
    /* Test case 4: Lookup non-existent resource */
    printf("\\nTest 4: Non-existent resource lookup\\n");
    res = jndi_lookup(&ctx, "nonexistent/resource");
    if (res == NULL) {
        printf("Correctly returned NULL for non-existent resource\\n");
    }
    
    /* Test case 5: Invalid name with URL scheme (should fail) */
    printf("\\nTest 5: Invalid name with URL scheme\\n");
    if (jndi_bind(&ctx, "ldap://attacker.com/exploit", RESOURCE_TYPE_STRING, "malicious") != 0) {
        printf("Correctly rejected URL scheme in name\\n");
    }
    
    printf("\\nAll tests completed successfully!\\n");
    
    /* Cleanup */
    jndi_cleanup(&ctx);
    
    return 0;
}
