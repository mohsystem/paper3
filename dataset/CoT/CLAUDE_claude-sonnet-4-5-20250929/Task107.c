
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_JNDI_LENGTH 256
#define MAX_CONTEXTS 3

// Allowed JNDI contexts
static const char* ALLOWED_CONTEXTS[MAX_CONTEXTS] = {
    "java:comp/env/jdbc/",
    "java:comp/env/jms/",
    "java:comp/env/mail/"
};

// Validates if character is safe for JNDI name
bool is_safe_char(char c) {
    return isalnum(c) || c == '/' || c == '_' || c == '-' || c == ':';
}

// Validates JNDI name format
bool validate_jndi_format(const char* jndi_name) {
    if (jndi_name == NULL || strlen(jndi_name) == 0) {
        return false;
    }
    
    for (size_t i = 0; i < strlen(jndi_name); i++) {
        if (!is_safe_char(jndi_name[i])) {
            return false;
        }
    }
    
    return true;
}

// Checks if JNDI name starts with allowed context
bool is_allowed_context(const char* jndi_name) {
    for (int i = 0; i < MAX_CONTEXTS; i++) {
        if (strncmp(jndi_name, ALLOWED_CONTEXTS[i], strlen(ALLOWED_CONTEXTS[i])) == 0) {
            return true;
        }
    }
    return false;
}

// Performs secure JNDI lookup with validation
void* perform_secure_jndi_lookup(const char* jndi_name) {
    if (jndi_name == NULL || strlen(jndi_name) == 0) {
        printf("Error: JNDI name cannot be null or empty\\n");
        return NULL;
    }
    
    // Validate JNDI name format
    if (!validate_jndi_format(jndi_name)) {
        printf("Error: Invalid JNDI name format: %s\\n", jndi_name);
        return NULL;
    }
    
    // Check if JNDI name starts with allowed context
    if (!is_allowed_context(jndi_name)) {
        printf("Error: JNDI context not allowed: %s\\n", jndi_name);
        return NULL;
    }
    
    // Simulate JNDI lookup (C doesn't have native JNDI)
    printf("Successfully validated JNDI name: %s\\n", jndi_name);
    return (void*)1; // Simulated resource pointer
}

// Validates if a JNDI name is safe and allowed
bool is_valid_jndi_name(const char* jndi_name) {
    if (!validate_jndi_format(jndi_name)) {
        return false;
    }
    
    return is_allowed_context(jndi_name);
}

int main() {
    printf("=== Secure JNDI Lookup Program ===\\n\\n");
    
    // Test Case 1: Valid JDBC lookup
    printf("Test 1: Valid JDBC lookup\\n");
    perform_secure_jndi_lookup("java:comp/env/jdbc/myDataSource");
    
    // Test Case 2: Valid JMS lookup
    printf("\\nTest 2: Valid JMS lookup\\n");
    perform_secure_jndi_lookup("java:comp/env/jms/myQueue");
    
    // Test Case 3: Invalid characters (injection attempt)
    printf("\\nTest 3: Invalid characters (security test)\\n");
    perform_secure_jndi_lookup("java:comp/env/jdbc/../../../malicious");
    
    // Test Case 4: Unauthorized context
    printf("\\nTest 4: Unauthorized context\\n");
    perform_secure_jndi_lookup("rmi://malicious.server/Exploit");
    
    // Test Case 5: Empty JNDI name
    printf("\\nTest 5: Empty JNDI name\\n");
    perform_secure_jndi_lookup("");
    
    // Test validation function
    printf("\\n=== JNDI Name Validation Tests ===\\n");
    printf("Valid name: %d\\n", is_valid_jndi_name("java:comp/env/jdbc/testDB"));
    printf("Invalid name: %d\\n", is_valid_jndi_name("ldap://evil.com/exploit"));
    
    return 0;
}
