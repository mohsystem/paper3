// Requires OpenLDAP development libraries.
// On Debian/Ubuntu: sudo apt-get install libldap2-dev
// On RedHat/CentOS: sudo yum install openldap-devel
// Compile with: gcc -o Task68 Task68.c -lldap -llber

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>

void search_ldap_user(const char* base_dn, const char* username) {
    LDAP* ld = NULL;
    const char* ldap_host = "localhost";
    const int ldap_port = 389;
    char ldap_uri[256];
    snprintf(ldap_uri, sizeof(ldap_uri), "ldap://%s:%d", ldap_host, ldap_port);

    // Initialize LDAP session
    int rc = ldap_initialize(&ld, ldap_uri);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "Error: ldap_initialize failed: %s\n", ldap_err2string(rc));
        return;
    }

    // Set LDAP protocol version to 3
    int ldap_version = LDAP_VERSION3;
    rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "Error: ldap_set_option failed to set version 3: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }

    // Bind anonymously
    rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "Error: ldap_simple_bind_s failed: %s\n", ldap_err2string(rc));
        ldap_unbind_ext_s(ld, NULL, NULL);
        return;
    }

    // Construct search filter
    char filter[256];
    snprintf(filter, sizeof(filter), "(&(objectClass=person)(uid=%s))", username);

    LDAPMessage* res = NULL;
    rc = ldap_search_ext_s(ld, base_dn, LDAP_SCOPE_SUBTREE, filter, NULL, 0, NULL, NULL, NULL, 0, &res);
    
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "LDAP Search Error: %s\n", ldap_err2string(rc));
    } else {
        int num_entries = ldap_count_entries(ld, res);
        if (num_entries == 0) {
            printf("User '%s' not found in '%s'.\n\n", username, base_dn);
        } else {
            LDAPMessage* entry;
            for (entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry)) {
                char* dn = ldap_get_dn(ld, entry);
                printf("Found entry: %s\n", dn);
                ldap_memfree(dn);

                BerElement* ber = NULL;
                char* attr;
                for (attr = ldap_first_attribute(ld, entry, &ber); attr != NULL; attr = ldap_next_attribute(ld, entry, ber)) {
                    printf("  Attribute: %s\n", attr);
                    struct berval** vals = ldap_get_values_len(ld, entry, attr);
                    if (vals != NULL) {
                        for (int i = 0; vals[i] != NULL; i++) {
                            printf("    Value: %s\n", vals[i]->bv_val);
                        }
                        ldap_value_free_len(vals);
                    }
                    ldap_memfree(attr);
                }
                if (ber != NULL) ber_free(ber, 1);
            }
             printf("\n");
        }
    }

    if (res != NULL) ldap_msgfree(res);
    ldap_unbind_ext_s(ld, NULL, NULL);
}

int main(int argc, char* argv[]) {
    if (argc > 1 && (argc - 1) % 2 == 0) {
        printf("Running test cases from command line arguments...\n");
        for (int i = 1; i < argc; i += 2) {
            const char* base_dn = argv[i];
            const char* username = argv[i+1];
            printf("--- Test Case %d: base_dn=%s, username=%s ---\n", (i/2 + 1), base_dn, username);
            search_ldap_user(base_dn, username);
        }
    } else {
        printf("Running predefined test cases...\n");
        // Test Case 1: A standard valid user search
        printf("--- Test Case 1: Find user 'jdoe' in 'dc=example,dc=com' ---\n");
        search_ldap_user("dc=example,dc=com", "jdoe");

        // Test Case 2: Another valid user
        printf("--- Test Case 2: Find user 'asmith' in 'dc=example,dc=com' ---\n");
        search_ldap_user("dc=example,dc=com", "asmith");

        // Test Case 3: A user that does not exist
        printf("--- Test Case 3: Find non-existent user 'nobody' ---\n");
        search_ldap_user("dc=example,dc=com", "nobody");

        // Test Case 4: A search in a non-existent base DN
        printf("--- Test Case 4: Search in a non-existent base DN ---\n");
        search_ldap_user("dc=nonexistent,dc=org", "jdoe");

        // Test Case 5: A user in a different, valid base DN
        printf("--- Test Case 5: Find user 'bcarter' in 'dc=test,dc=org' ---\n");
        search_ldap_user("dc=test,dc=org", "bcarter");

        printf("\nUsage for custom tests: ./Task68 <baseDn1> <user1> <baseDn2> <user2> ...\n");
        printf("Example: ./Task68 dc=example,dc=com jdoe dc=example,dc=com nonexistant\n");
    }

    return 0;
}