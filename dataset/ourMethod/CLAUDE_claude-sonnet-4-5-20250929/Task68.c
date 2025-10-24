
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>
#include <ctype.h>

#define MAX_USERNAME_LEN 64
#define MAX_DC_LEN 253
#define MAX_FILTER_LEN 256
#define MAX_BASEDN_LEN 512
#define MAX_RESULT_LEN 4096

/* Validate username - alphanumeric, underscore, hyphen, dot only */
int validate_username(const char* username) {
    size_t len;
    size_t i;
    
    if (username == NULL) {
        return 0;
    }
    
    len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LEN) {
        return 0;
    }
    
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && 
            username[i] != '.' && 
            username[i] != '_' && 
            username[i] != '-') {
            return 0;
        }
    }
    
    return 1;
}

/* Validate domain component */
int validate_dc(const char* dc) {
    size_t len;
    size_t i;
    int dot_count = 0;
    int component_len = 0;
    
    if (dc == NULL) {
        return 0;
    }
    
    len = strlen(dc);
    if (len == 0 || len > MAX_DC_LEN) {
        return 0;
    }
    
    for (i = 0; i < len; i++) {
        if (dc[i] == '.') {
            if (component_len == 0 || component_len > 63) {
                return 0;
            }
            component_len = 0;
            dot_count++;
        } else if (isalnum((unsigned char)dc[i]) || dc[i] == '-') {
            component_len++;
        } else {
            return 0;
        }
    }
    
    /* Check last component */
    if (component_len == 0 || component_len > 63) {
        return 0;
    }
    
    return 1;
}

/* Escape LDAP special characters to prevent injection */
char* escape_ldap_string(const char* input) {
    size_t i;
    size_t j = 0;
    size_t input_len;
    char* escaped = NULL;
    
    if (input == NULL) {
        return NULL;
    }
    
    input_len = strlen(input);
    
    /* Allocate maximum possible size (each char could be escaped) */
    escaped = (char*)calloc(input_len * 2 + 1, sizeof(char));
    if (escaped == NULL) {
        return NULL;
    }
    
    for (i = 0; i < input_len; i++) {
        switch (input[i]) {
            case '*':
            case '(':
            case ')':
            case '\\\\':\n            case '\\0':\n                escaped[j++] = '\\\\';\n                escaped[j++] = input[i];\n                break;\n            default:\n                escaped[j++] = input[i];\n                break;\n        }\n    }\n    \n    escaped[j] = '\\0';\n    return escaped;\n}\n\n/* Convert domain to LDAP base DN format */\nchar* domain_to_base_dn(const char* domain) {\n    size_t i;\n    size_t domain_len;\n    size_t start = 0;\n    char* base_dn = NULL;\n    char* escaped_component = NULL;\n    char component[64];\n    int first = 1;\n    size_t base_dn_pos = 0;\n    \n    if (domain == NULL) {\n        return NULL;\n    }\n    \n    domain_len = strlen(domain);\n    base_dn = (char*)calloc(MAX_BASEDN_LEN, sizeof(char));\n    if (base_dn == NULL) {\n        return NULL;\n    }\n    \n    for (i = 0; i <= domain_len; i++) {\n        if (domain[i] == '.' || domain[i] == '\\0') {\n            size_t component_len = i - start;\n            if (component_len >= sizeof(component)) {\n                free(base_dn);\n                return NULL;\n            }\n            \n            memcpy(component, domain + start, component_len);\n            component[component_len] = '\\0';\n            \n            escaped_component = escape_ldap_string(component);\n            if (escaped_component == NULL) {\n                free(base_dn);\n                return NULL;\n            }\n            \n            if (!first) {\n                if (base_dn_pos + 1 >= MAX_BASEDN_LEN) {\n                    free(escaped_component);\n                    free(base_dn);\n                    return NULL;\n                }\n                base_dn[base_dn_pos++] = ',';\n            }\n            \n            /* Check bounds before copying */\n            if (base_dn_pos + 3 + strlen(escaped_component) >= MAX_BASEDN_LEN) {\n                free(escaped_component);\n                free(base_dn);\n                return NULL;\n            }\n            \n            memcpy(base_dn + base_dn_pos, "dc=", 3);\n            base_dn_pos += 3;\n            strcpy(base_dn + base_dn_pos, escaped_component);\n            base_dn_pos += strlen(escaped_component);\n            \n            free(escaped_component);\n            escaped_component = NULL;\n            first = 0;\n            start = i + 1;\n        }\n    }\n    \n    base_dn[base_dn_pos] = '\\0';\n    return base_dn;\n}\n\n/* Search LDAP and return user information */\nchar* search_ldap(const char* username, const char* dc) {\n    LDAP* ld = NULL;\n    int result;\n    int version = LDAP_VERSION3;\n    char* base_dn = NULL;\n    char* escaped_username = NULL;\n    char filter[MAX_FILTER_LEN];\n    char* attrs[] = {"uid", "cn", "mail", "displayName", NULL};\n    LDAPMessage* search_result = NULL;\n    LDAPMessage* entry = NULL;\n    char* user_info = NULL;\n    struct timeval timeout;\n    char* dn = NULL;\n    BerElement* ber = NULL;\n    char* attr = NULL;\n    struct berval** vals = NULL;\n    size_t user_info_pos = 0;\n    int i;\n    \n    /* Validate inputs */\n    if (!validate_username(username)) {\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: Invalid username format", 255);\n        }\n        return user_info;\n    }\n    \n    if (!validate_dc(dc)) {\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: Invalid domain component format", 255);\n        }\n        return user_info;\n    }\n    \n    /* Initialize LDAP connection */\n    result = ldap_initialize(&ld, "ldap://localhost:389");\n    if (result != LDAP_SUCCESS) {\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: Failed to initialize LDAP connection", 255);\n        }\n        return user_info;\n    }\n    \n    /* Set LDAP version */\n    result = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &version);\n    if (result != LDAP_SUCCESS) {\n        ldap_unbind_ext_s(ld, NULL, NULL);\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: Failed to set LDAP protocol version", 255);\n        }\n        return user_info;\n    }\n    \n    /* Set timeout */\n    timeout.tv_sec = 10;\n    timeout.tv_usec = 0;\n    ldap_set_option(ld, LDAP_OPT_NETWORK_TIMEOUT, &timeout);\n    \n    /* Anonymous bind */\n    result = ldap_simple_bind_s(ld, NULL, NULL);\n    if (result != LDAP_SUCCESS) {\n        ldap_unbind_ext_s(ld, NULL, NULL);\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: LDAP bind failed", 255);\n        }\n        return user_info;\n    }\n    \n    /* Construct base DN */\n    base_dn = domain_to_base_dn(dc);\n    if (base_dn == NULL) {\n        ldap_unbind_ext_s(ld, NULL, NULL);\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: Failed to construct base DN", 255);\n        }\n        return user_info;\n    }\n    \n    /* Construct LDAP filter with escaped username */\n    escaped_username = escape_ldap_string(username);\n    if (escaped_username == NULL) {\n        free(base_dn);\n        ldap_unbind_ext_s(ld, NULL, NULL);\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: Failed to escape username", 255);\n        }\n        return user_info;\n    }\n    \n    /* Check filter length before construction */\n    if (strlen(escaped_username) + 7 >= MAX_FILTER_LEN) {\n        free(escaped_username);\n        free(base_dn);\n        ldap_unbind_ext_s(ld, NULL, NULL);\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: Username too long", 255);\n        }\n        return user_info;\n    }\n    \n    snprintf(filter, MAX_FILTER_LEN, "(uid=%s)", escaped_username);\n    free(escaped_username);\n    escaped_username = NULL;\n    \n    /* Perform LDAP search */\n    result = ldap_search_ext_s(\n        ld,\n        base_dn,\n        LDAP_SCOPE_SUBTREE,\n        filter,\n        attrs,\n        0,\n        NULL,\n        NULL,\n        &timeout,\n        1000,\n        &search_result\n    );\n    \n    free(base_dn);\n    base_dn = NULL;\n    \n    if (result != LDAP_SUCCESS) {\n        if (search_result != NULL) {\n            ldap_msgfree(search_result);\n        }\n        ldap_unbind_ext_s(ld, NULL, NULL);\n        user_info = (char*)calloc(256, sizeof(char));\n        if (user_info != NULL) {\n            strncpy(user_info, "Error: LDAP search failed", 255);\n        }\n        return user_info;\n    }\n    \n    /* Allocate result buffer */\n    user_info = (char*)calloc(MAX_RESULT_LEN, sizeof(char));\n    if (user_info == NULL) {\n        ldap_msgfree(search_result);\n        ldap_unbind_ext_s(ld, NULL, NULL);\n        return NULL;\n    }\n    \n    /* Check if any entries found */\n    if (ldap_count_entries(ld, search_result) == 0) {\n        strncpy(user_info, "No user found", MAX_RESULT_LEN - 1);\n        ldap_msgfree(search_result);\n        ldap_unbind_ext_s(ld, NULL, NULL);\n        return user_info;\n    }\n    \n    /* Process first entry */\n    entry = ldap_first_entry(ld, search_result);\n    if (entry != NULL) {\n        dn = ldap_get_dn(ld, entry);\n        if (dn != NULL) {\n            int len = snprintf(user_info + user_info_pos, \n                             MAX_RESULT_LEN - user_info_pos, \n                             "DN: %s\\n", dn);\n            if (len > 0 && (size_t)len < MAX_RESULT_LEN - user_info_pos) {\n                user_info_pos += len;\n            }\n            ldap_memfree(dn);\n        }\n        \n        /* Iterate through attributes */\n        attr = ldap_first_attribute(ld, entry, &ber);\n        while (attr != NULL && user_info_pos < MAX_RESULT_LEN - 100) {\n            vals = ldap_get_values_len(ld, entry, attr);\n            if (vals != NULL) {\n                for (i = 0; vals[i] != NULL && user_info_pos < MAX_RESULT_LEN - 100; i++) {\n                    int len = snprintf(user_info + user_info_pos,\n                                     MAX_RESULT_LEN - user_info_pos,\n                                     "%s: %.*s\\n",\n                                     attr,\n                                     (int)vals[i]->bv_len,\n                                     vals[i]->bv_val);\n                    if (len > 0 && (size_t)len < MAX_RESULT_LEN - user_info_pos) {\n                        user_info_pos += len;\n                    }\n                }\n                ldap_value_free_len(vals);\n            }\n            ldap_memfree(attr);\n            attr = ldap_next_attribute(ld, entry, ber);\n        }\n        \n        if (ber != NULL) {\n            ber_free(ber, 0);\n        }\n    }\n    \n    user_info[MAX_RESULT_LEN - 1] = '\\0';
    ldap_msgfree(search_result);
    ldap_unbind_ext_s(ld, NULL, NULL);
    
    return user_info;
}

int main(int argc, char* argv[]) {
    const char* test_usernames[] = {"testuser1", "john.doe", "admin", "user_123", "validuser"};
    const char* test_dcs[] = {"example.com", "corp.example.com", "test.local", "domain.org", "sub.domain.net"};
    int i;
    char* result = NULL;
    
    if (argc == 3) {
        /* Use command line arguments */
        result = search_ldap(argv[1], argv[2]);
        if (result != NULL) {
            printf("%s\\n", result);
            free(result);
        }
    } else {
        /* Run test cases */
        printf("Running test cases...\\n");
        for (i = 0; i < 5; i++) {
            printf("\\n--- Testing username: %s, dc: %s ---\\n", 
                   test_usernames[i], test_dcs[i]);
            result = search_ldap(test_usernames[i], test_dcs[i]);
            if (result != NULL) {
                printf("%s\\n", result);
                free(result);
                result = NULL;
            }
        }
    }
    
    return 0;
}
