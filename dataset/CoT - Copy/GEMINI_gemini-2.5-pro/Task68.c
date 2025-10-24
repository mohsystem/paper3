/*
NOTE: Requires the OpenLDAP development library.
On Debian/Ubuntu: sudo apt-get install libldap2-dev
On RHEL/CentOS: sudo yum install openldap-devel

Compile with: gcc -o task68_c task68.c -lldap -llber

NOTE: This program requires a running LDAP server on localhost:389 with appropriate data.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>

// Helper function to escape special characters for an LDAP filter.
// This is a crucial security measure to prevent LDAP Injection.
char* escapeLdapFilter(const char* input) {
    if (!input) return NULL;
    size_t input_len = strlen(input);
    // Worst case: every character is escaped ("\xx"), needing 3x space.
    char* escaped = malloc(input_len * 3 + 1); 
    if (!escaped) {
        perror("malloc failed for escaping");
        return NULL;
    }

    char* p = escaped;
    for (size_t i = 0; i < input_len; ++i) {
        char c = input[i];
        switch (c) {
            case '\\': strcpy(p, "\\5c"); p += 3; break;
            case '*':  strcpy(p, "\\2a"); p += 3; break;
            case '(':  strcpy(p, "\\28"); p += 3; break;
            case ')':  strcpy(p, "\\29"); p += 3; break;
            case '\0': strcpy(p, "\\00"); p += 3; break;
            default:   *p++ = c;
        }
    }
    *p = '\0';
    return escaped;
}

// Helper function to build the Base DN string from a comma-separated component.
char* buildBaseDN(const char* domainComponent) {
    if (!domainComponent) return NULL;
    // Allocate generous memory: "dc=" + length + commas replaced with ",dc="
    size_t required_len = strlen(domainComponent) * 4 + 4;
    char* baseDN = (char*)malloc(required_len);
    if (!baseDN) {
        perror("malloc failed for base DN");
        return NULL;
    }
    
    strcpy(baseDN, "dc=");
    char* rest = strdup(domainComponent); // Work on a copy as strtok_r modifies it.
    if (!rest) { free(baseDN); return NULL; }
    char* original_rest = rest;

    char* token = strtok_r(rest, ",", &rest);
    while (token != NULL) {
        strcat(baseDN, token);
        token = strtok_r(NULL, ",", &rest);
        if (token != NULL) {
            strcat(baseDN, ",dc=");
        }
    }
    free(original_rest);
    return baseDN;
}


void searchLdapUser(const char* username, const char* domainComponent) {
    LDAP *ld = NULL;
    LDAPMessage *res = NULL;
    const char* ldap_uri = "ldap://localhost:389"; // Use ldaps:// for production
    int rc = 0;

    // --- 1. Initialize LDAP session ---
    rc = ldap_initialize(&ld, ldap_uri);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "Error: ldap_initialize failed: %s\n", ldap_err2string(rc));
        return;
    }

    int protocol_version = LDAP_VERSION3;
    rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &protocol_version);
    if (rc != LDAP_SUCCESS) {
        ldap_perror(ld, "ldap_set_option");
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }

    // --- 2. Bind to the server (anonymous bind for this example) ---
    rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        ldap_perror(ld, "ldap_simple_bind_s");
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }

    // --- 3. Construct the search base and filter safely ---
    char* searchBase = buildBaseDN(domainComponent);
    char* escapedUsername = escapeLdapFilter(username);
    if (!searchBase || !escapedUsername) {
        free(searchBase);
        free(escapedUsername);
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }
    
    // Max filter size: "(uid=" + escaped_len + ")" + 1
    size_t filter_len = strlen(escapedUsername) + 7;
    char* searchFilter = (char*)malloc(filter_len);
    if (!searchFilter) {
        perror("malloc failed for filter");
        free(searchBase);
        free(escapedUsername);
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }
    snprintf(searchFilter, filter_len, "(uid=%s)", escapedUsername);

    // Specify attributes to return to avoid exposing sensitive data
    char* attrs[] = { "cn", "sn", "mail", "uid", NULL };

    // --- 4. Perform the synchronous search ---
    printf("Searching for user '%s' in base '%s'...\n", username, searchBase);
    rc = ldap_search_ext_s(ld, searchBase, LDAP_SCOPE_SUBTREE, searchFilter, attrs, 0, NULL, NULL, NULL, 0, &res);

    if (rc != LDAP_SUCCESS && rc != LDAP_NO_SUCH_OBJECT) {
        fprintf(stderr, "LDAP search error: %s\n", ldap_err2string(rc));
    }
    
    if (ldap_count_entries(ld, res) == 0) {
        printf("User '%s' not found.\n", username);
    } else {
        printf("Search results:\n");
        LDAPMessage* entry;
        for (entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry)) {
            char* dn = ldap_get_dn(ld, entry);
            if (dn) {
                printf(">> DN: %s\n", dn);
                ldap_memfree(dn);
            }
            BerElement* ber = NULL;
            char* attr;
            for (attr = ldap_first_attribute(ld, entry, &ber); attr != NULL; attr = ldap_next_attribute(ld, entry, ber)) {
                struct berval** vals = ldap_get_values_len(ld, entry, attr);
                if (vals) {
                    printf("   %s: ", attr);
                    for (int i = 0; vals[i] != NULL; i++) {
                        printf("%s ", vals[i]->bv_val);
                    }
                    printf("\n");
                    ldap_value_free_len(vals);
                }
                ldap_memfree(attr);
            }
            if (ber) ber_free(ber, 0);
        }
    }

    // --- 5. Clean up all allocated resources ---
    free(searchBase);
    free(escapedUsername);
    free(searchFilter);
    if (res) ldap_msgfree(res);
    ldap_unbind_ext_s(ld, NULL, NULL);
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        printf("--- Running with Command Line Arguments ---\n");
        searchLdapUser(argv[1], argv[2]);
    } else {
        printf("Usage: %s <username> <domain_component_string>\n", argv[0]);
        printf("Example: %s jdoe example,com\n\n", argv[0]);
        printf("NOTE: This program requires a running LDAP server on localhost:389 with appropriate data.\n\n");
        
        // --- 5 Hardcoded Test Cases ---
        printf("--- Running 5 Hardcoded Test Cases ---\n");

        printf("\n--- Test Case 1: Valid User ---\n");
        searchLdapUser("jdoe", "example,com");

        printf("\n--- Test Case 2: Non-existent User ---\n");
        searchLdapUser("nonexistentuser", "example,com");

        printf("\n--- Test Case 3: User in a different DC ---\n");
        searchLdapUser("asmith", "test,org");
        
        printf("\n--- Test Case 4: Potential Injection Attempt ---\n");
        searchLdapUser("jdoe)(uid=*", "example,com");
        
        printf("\n--- Test Case 5: User with special chars ---\n");
        searchLdapUser("user*with(special)chars", "example,com");
    }
    return 0;
}