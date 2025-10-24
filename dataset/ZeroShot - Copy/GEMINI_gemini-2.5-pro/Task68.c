#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ldap.h>

// NOTE: This code requires the OpenLDAP client development libraries.
// On Debian/Ubuntu: sudo apt-get install libldap2-dev
// On RHEL/CentOS: sudo yum install openldap-devel
// To compile: gcc -o Task68 Task68.c -lldap

// A simple dynamic string buffer for building the result string
typedef struct {
    char *str;
    size_t len;
    size_t capacity;
} StringBuilder;

void sb_init(StringBuilder *sb) {
    sb->capacity = 256;
    sb->len = 0;
    sb->str = (char*)malloc(sb->capacity);
    if (sb->str) { sb->str[0] = '\0'; }
}

void sb_append(StringBuilder *sb, const char *text) {
    if (!sb->str || !text) return;
    size_t text_len = strlen(text);
    if (sb->len + text_len + 1 > sb->capacity) {
        size_t new_capacity = sb->capacity;
        while(new_capacity < sb->len + text_len + 1) { new_capacity *= 2; }
        char *new_str = (char*)realloc(sb->str, new_capacity);
        if (!new_str) return;
        sb->str = new_str;
        sb->capacity = new_capacity;
    }
    strcat(sb->str, text);
    sb->len += text_len;
}

char* searchLdapUser(const char* dcString, const char* username) {
    LDAP* ld = NULL;
    char ldap_uri[256];
    int rc = 0;

    StringBuilder sb;
    sb_init(&sb);

    snprintf(ldap_uri, sizeof(ldap_uri), "ldap://localhost:389");

    rc = ldap_initialize(&ld, ldap_uri);
    if (rc != LDAP_SUCCESS) {
        sb_append(&sb, "ldap_initialize failed: ");
        sb_append(&sb, ldap_err2string(rc));
        return sb.str;
    }

    int protocol_version = LDAP_VERSION3;
    rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &protocol_version);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, NULL, NULL);
        sb_append(&sb, "ldap_set_option failed: ");
        sb_append(&sb, ldap_err2string(rc));
        return sb.str;
    }

    rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, NULL, NULL);
        sb_append(&sb, "ldap_simple_bind_s failed: ");
        sb_append(&sb, ldap_err2string(rc));
        return sb.str;
    }

    char* searchBase = (char*)malloc(strlen(dcString) * 4 + 4);
    if (!searchBase) { ldap_unbind_ext_s(ld, NULL, NULL); sb_append(&sb, "malloc failed"); return sb.str; }
    strcpy(searchBase, "dc=");
    char* dc_copy = strdup(dcString);
    char* rest = dc_copy;
    char* token;
    int first = 1;
    while ((token = strtok_r(rest, ",", &rest))) {
        if (!first) { strcat(searchBase, ",dc="); }
        strcat(searchBase, token);
        first = 0;
    }
    free(dc_copy);

    BerValue berUsername;
    berUsername.bv_val = (char*)username;
    berUsername.bv_len = strlen(username);
    BerValue* escaped_val = NULL;
    char* searchFilter = NULL;

    rc = ldap_bv2escaped_filter_value_x(ld, &berUsername, &escaped_val, 0);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(searchBase);
        sb_append(&sb, "Failed to escape filter value");
        return sb.str;
    }

    searchFilter = (char*)malloc(escaped_val->bv_len + 6);
    if (!searchFilter) {
        ldap_unbind_ext_s(ld, NULL, NULL);
        free(searchBase);
        ldap_memfree(escaped_val->bv_val); ldap_memfree(escaped_val);
        sb_append(&sb, "malloc failed");
        return sb.str;
    }
    sprintf(searchFilter, "(cn=%s)", escaped_val->bv_val);

    sb_append(&sb, "Searching for user '"); sb_append(&sb, username);
    sb_append(&sb, "'\nBase DN: "); sb_append(&sb, searchBase);
    sb_append(&sb, "\nFilter: "); sb_append(&sb, searchFilter);
    sb_append(&sb, "\n\n");

    LDAPMessage* res = NULL;
    rc = ldap_search_ext_s(ld, searchBase, LDAP_SCOPE_SUBTREE, searchFilter, NULL, 0, NULL, NULL, NULL, 0, &res);

    if (rc != LDAP_SUCCESS && rc != LDAP_SIZELIMIT_EXCEEDED) {
        sb_append(&sb, "ldap_search_ext_s failed: ");
        sb_append(&sb, ldap_err2string(rc));
    } else {
        if (ldap_count_entries(ld, res) == 0) {
            sb_append(&sb, "User '"); sb_append(&sb, username); sb_append(&sb, "' not found.\n");
        } else {
            LDAPMessage* entry;
            for (entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry)) {
                char* dn = ldap_get_dn(ld, entry);
                if(dn) { sb_append(&sb, "Found user DN: "); sb_append(&sb, dn); sb_append(&sb, "\n"); ldap_memfree(dn); }
                
                BerElement* ber = NULL;
                char* attr;
                for (attr = ldap_first_attribute(ld, entry, &ber); attr != NULL; attr = ldap_next_attribute(ld, entry, ber)) {
                    berval** vals = ldap_get_values_len(ld, entry, attr);
                    if (vals != NULL) {
                        for (int i = 0; vals[i] != NULL; i++) {
                            sb_append(&sb, attr); sb_append(&sb, ": "); sb_append(&sb, vals[i]->bv_val); sb_append(&sb, "\n");
                        }
                        ldap_value_free_len(vals);
                    }
                    ldap_memfree(attr);
                }
                if (ber != NULL) ber_free(ber, 1);
            }
        }
    }

    ldap_memfree(escaped_val->bv_val); ldap_memfree(escaped_val);
    free(searchFilter); free(searchBase);
    if (res != NULL) ldap_msgfree(res);
    ldap_unbind_ext_s(ld, NULL, NULL);

    return sb.str;
}

void run_test(int test_num, const char* username, const char* dcString) {
    printf("----------- Test Case %d -----------\n", test_num);
    char* result = searchLdapUser(dcString, username);
    if (result) {
        printf("%s\n", result);
        free(result);
    }
}

int main(int argc, char* argv[]) {
    printf("NOTE: This program requires the OpenLDAP client development libraries.\n");
    printf("NOTE: This program also requires an LDAP server running on localhost:389.\n");
    printf("For tests to succeed, it should have a base DN like 'dc=example,dc=com'\n");
    printf("and users with cn='jdoe', 'test(user)', 'star*user', etc.\n\n");

    run_test(1, "jdoe", "example,com");
    run_test(2, "nonexistent", "example,com");
    run_test(3, "test(user)", "example,com");
    run_test(4, "star*user", "example,com");
    run_test(5, "*", "example,com");

    return 0;
}