
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    #include <winldap.h>
    #pragma comment(lib, "wldap32.lib")
#else
    #include <ldap.h>
#endif

char* search_ldap_user(const char* domain_component, const char* username) {
    static char result[4096];
    result[0] = '\\0';
    LDAP* ldap = NULL;
    
    // Initialize LDAP connection
    #ifdef _WIN32
    ldap = ldap_init("localhost", 389);
    #else
    int rc = ldap_initialize(&ldap, "ldap://localhost:389");
    if (rc != LDAP_SUCCESS) {
        sprintf(result, "Error: Failed to initialize LDAP connection");
        return result;
    }
    #endif
    
    if (ldap == NULL) {
        sprintf(result, "Error: Failed to initialize LDAP connection");
        return result;
    }
    
    // Set LDAP version to 3
    int version = LDAP_VERSION3;
    ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &version);
    
    // Bind anonymously
    int bind_result = ldap_simple_bind_s(ldap, NULL, NULL);
    if (bind_result != LDAP_SUCCESS) {
        sprintf(result, "Error: Failed to bind to LDAP server - %s", ldap_err2string(bind_result));
        ldap_unbind_s(ldap);
        return result;
    }
    
    // Construct base DN
    char base_dn[256];
    sprintf(base_dn, "dc=%s", domain_component);
    char* p = base_dn;
    while ((p = strchr(p, '.')) != NULL) {
        memmove(p + 4, p + 1, strlen(p));
        memcpy(p, ",dc=", 4);
        p += 4;
    }
    
    // Construct search filter
    char search_filter[256];
    sprintf(search_filter, "(uid=%s)", username);
    
    // Perform search
    LDAPMessage* search_result = NULL;
    int search_rc = ldap_search_s(ldap, base_dn, LDAP_SCOPE_SUBTREE,
                                  search_filter, NULL, 0, &search_result);
    
    if (search_rc == LDAP_SUCCESS && search_result != NULL) {
        int count = ldap_count_entries(ldap, search_result);
        
        if (count > 0) {
            LDAPMessage* entry = ldap_first_entry(ldap, search_result);
            char* dn = ldap_get_dn(ldap, entry);
            sprintf(result, "User found: %s\\nAttributes:\\n", dn);
            ldap_memfree(dn);
            
            BerElement* ber = NULL;
            char* attr = ldap_first_attribute(ldap, entry, &ber);
            while (attr != NULL) {
                char** vals = ldap_get_values(ldap, entry, attr);
                if (vals != NULL) {
                    char temp[256];
                    sprintf(temp, "  %s: %s\\n", attr, vals[0]);
                    strcat(result, temp);
                    ldap_value_free(vals);
                }
                ldap_memfree(attr);
                attr = ldap_next_attribute(ldap, entry, ber);
            }
            if (ber != NULL) ber_free(ber, 0);
        } else {
            sprintf(result, "User not found");
        }
        
        ldap_msgfree(search_result);
    } else {
        sprintf(result, "Error: Search failed - %s", ldap_err2string(search_rc));
    }
    
    ldap_unbind_s(ldap);
    return result;
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        printf("%s\\n", search_ldap_user(argv[1], argv[2]));
    } else {
        // Test cases
        printf("Test Case 1:\\n%s\\n\\n---\\n\\n", search_ldap_user("example.com", "john.doe"));
        printf("Test Case 2:\\n%s\\n\\n---\\n\\n", search_ldap_user("company.org", "alice"));
        printf("Test Case 3:\\n%s\\n\\n---\\n\\n", search_ldap_user("test.local", "bob.smith"));
        printf("Test Case 4:\\n%s\\n\\n---\\n\\n", search_ldap_user("domain.net", "admin"));
        printf("Test Case 5:\\n%s\\n", search_ldap_user("mycompany.com", "testuser"));
    }
    
    return 0;
}
