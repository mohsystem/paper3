
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>

char* searchLDAP(const char* dcComponent, const char* username) {
    static char result[4096];
    memset(result, 0, sizeof(result));
    LDAP* ldap = NULL;
    
    // Initialize LDAP connection
    int rc = ldap_initialize(&ldap, "ldap://localhost:389");
    if (rc != LDAP_SUCCESS) {
        snprintf(result, sizeof(result), "Error: Failed to initialize LDAP connection");
        return result;
    }
    
    // Set LDAP version to 3
    int version = LDAP_VERSION3;
    ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &version);
    
    // Anonymous bind
    rc = ldap_simple_bind_s(ldap, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ldap, NULL, NULL);
        snprintf(result, sizeof(result), "Error: Failed to bind to LDAP server");
        return result;
    }
    
    // Construct base DN from DC component
    char baseDN[256];
    char dcCopy[256];
    strncpy(dcCopy, dcComponent, sizeof(dcCopy) - 1);
    snprintf(baseDN, sizeof(baseDN), "dc=");
    
    char* token = strtok(dcCopy, ".");
    int first = 1;
    while (token != NULL) {
        if (!first) {
            strcat(baseDN, ",dc=");
        }
        strcat(baseDN, token);
        first = 0;
        token = strtok(NULL, ".");
    }
    
    // Construct LDAP search filter (VULNERABLE TO INJECTION)
    char searchFilter[256];
    snprintf(searchFilter, sizeof(searchFilter), "(uid=%s)", username);
    
    // Perform search
    LDAPMessage* searchResult = NULL;
    rc = ldap_search_ext_s(
        ldap,
        baseDN,
        LDAP_SCOPE_SUBTREE,
        searchFilter,
        NULL,
        0,
        NULL,
        NULL,
        NULL,
        0,
        &searchResult
    );
    
    if (rc == LDAP_SUCCESS && searchResult) {
        LDAPMessage* entry = ldap_first_entry(ldap, searchResult);
        
        if (entry) {
            int offset = 0;
            while (entry && offset < sizeof(result) - 100) {
                char* dn = ldap_get_dn(ldap, entry);
                if (dn) {
                    offset += snprintf(result + offset, sizeof(result) - offset, "DN: %s\\n", dn);
                    ldap_memfree(dn);
                }
                
                BerElement* ber = NULL;
                char* attr = ldap_first_attribute(ldap, entry, &ber);
                
                while (attr && offset < sizeof(result) - 100) {
                    struct berval** vals = ldap_get_values_len(ldap, entry, attr);
                    if (vals) {
                        for (int i = 0; vals[i] != NULL && offset < sizeof(result) - 100; i++) {
                            offset += snprintf(result + offset, sizeof(result) - offset, 
                                             "%s: %.*s\\n", attr, (int)vals[i]->bv_len, vals[i]->bv_val);
                        }
                        ldap_value_free_len(vals);
                    }
                    ldap_memfree(attr);
                    attr = ldap_next_attribute(ldap, entry, ber);
                }
                
                if (ber) ber_free(ber, 0);
                offset += snprintf(result + offset, sizeof(result) - offset, "\\n");
                entry = ldap_next_entry(ldap, entry);
            }
        } else {
            snprintf(result, sizeof(result), "No user found");
        }
        
        ldap_msgfree(searchResult);
    } else {
        snprintf(result, sizeof(result), "Error: Search failed - %s", ldap_err2string(rc));
    }
    
    ldap_unbind_ext_s(ldap, NULL, NULL);
    return result;
}

int main(int argc, char* argv[]) {
    // Test cases
    printf("Test Case 1:\\n%s\\n---\\n", searchLDAP("example.com", "john.doe"));
    printf("Test Case 2:\\n%s\\n---\\n", searchLDAP("company.org", "admin"));
    printf("Test Case 3:\\n%s\\n---\\n", searchLDAP("test.local", "user123"));
    printf("Test Case 4:\\n%s\\n---\\n", searchLDAP("domain.net", "testuser"));
    printf("Test Case 5:\\n%s\\n---\\n", searchLDAP("myorg.com", "employee"));
    
    // Command line argument handling
    if (argc >= 3) {
        printf("\\nCommand Line Arguments:\\n%s\\n", searchLDAP(argv[1], argv[2]));
    }
    
    return 0;
}
