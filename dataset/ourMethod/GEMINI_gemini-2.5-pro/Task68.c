#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

// Note: This program requires the OpenLDAP client libraries.
// On Debian/Ubuntu: sudo apt-get install libldap2-dev
// On RedHat/CentOS: sudo yum install openldap-devel
// Compile with: gcc task.c -o task -lldap
#include <ldap.h>

/**
 * @brief Validates a string against a POSIX extended regex pattern.
 * @param str The string to validate.
 * @param pattern The regex pattern.
 * @return 1 for a match, 0 for no match or error.
 */
static int validate_string(const char* str, const char* pattern) {
    regex_t regex;
    int reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }
    reti = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);
    return !reti; // 0 means match, so invert it
}

/**
 * @brief Escapes special characters for an LDAP filter according to RFC 4515.
 * @param input The raw string to escape.
 * @return A new, dynamically allocated escaped string. Must be freed by caller.
 */
static char* escapeLdapFilter(const char* input) {
    size_t input_len = strlen(input);
    // Worst case: every character is escaped ('c' -> '\\xx'), so 3x the size + null terminator
    char* escaped = (char*)malloc(input_len * 3 + 1);
    if (!escaped) return NULL;

    char* p = escaped;
    for (size_t i = 0; i < input_len; ++i) {
        char c = input[i];
        switch (c) {
            case '*': strcpy(p, "\\2a"); p += 3; break;
            case '(': strcpy(p, "\\28"); p += 3; break;
            case ')': strcpy(p, "\\29"); p += 3; break;
            case '\\': strcpy(p, "\\5c"); p += 3; break;
            case '\0': strcpy(p, "\\00"); p += 3; break;
            default: *p++ = c;
        }
    }
    *p = '\0';
    return escaped;
}

/**
 * @brief Extracts user info from an LDAP directory based on user input.
 * @param userInput A string in the format "username@domain.com".
 */
void findUserInLdap(const char* userInput) {
    if (userInput == NULL || *userInput == '\0') {
        printf("Error: Input cannot be null or empty.\n");
        return;
    }

    char* userInputCopy = strdup(userInput);
    if (!userInputCopy) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    char* at_symbol = strchr(userInputCopy, '@');
    if (at_symbol == NULL || at_symbol == userInputCopy || *(at_symbol + 1) == '\0') {
        printf("Error: Invalid input format. Expected 'username@domain.com'.\n");
        free(userInputCopy);
        return;
    }
    *at_symbol = '\0';
    char* username = userInputCopy;
    char* domain = at_symbol + 1;
    
    if (!validate_string(domain, "^[a-zA-Z0-9.-]+$") || !validate_string(username, "^[a-zA-Z0-9_.-]+$")) {
        printf("Error: Username or domain contains invalid characters.\n");
        free(userInputCopy);
        return;
    }

    char* domain_copy_for_dn = strdup(domain);
    char baseDn[256] = "";
    char* token = strtok(domain_copy_for_dn, ".");
    int first = 1;
    while(token != NULL) {
        char component[128];
        snprintf(component, sizeof(component), "%sdc=%s", (first ? "" : ","), token);
        if (strlen(baseDn) + strlen(component) < sizeof(baseDn) - 1) {
            strcat(baseDn, component);
        } else {
            printf("Error: Constructed DN is too long.\n");
            free(domain_copy_for_dn);
            free(userInputCopy);
            return;
        }
        first = 0;
        token = strtok(NULL, ".");
    }
    free(domain_copy_for_dn);
    
    char* escapedUsername = escapeLdapFilter(username);
    if (!escapedUsername) {
        printf("Error: Memory allocation failed.\n");
        free(userInputCopy);
        return;
    }
    char searchFilter[512];
    snprintf(searchFilter, sizeof(searchFilter), "(uid=%s)", escapedUsername);
    
    free(escapedUsername);
    free(userInputCopy);

    LDAP* ld = NULL;
    int rc = ldap_initialize(&ld, "ldap://localhost:389");
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "Error: ldap_initialize failed: %s\n", ldap_err2string(rc));
        return;
    }

    int ldap_version = LDAP_VERSION3;
    rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldap_version);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "Error: ldap_set_option failed: %s\n", ldap_err2string(rc));
        ldap_destroy(ld);
        return;
    }

    rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "Error: ldap_simple_bind_s failed: %s\n", ldap_err2string(rc));
        ldap_destroy(ld);
        return;
    }

    printf("Searching for user in '%s'\n", baseDn);
    printf("Using filter: %s\n", searchFilter);

    LDAPMessage* res = NULL;
    rc = ldap_search_ext_s(ld, baseDn, LDAP_SCOPE_SUBTREE, searchFilter, NULL, 0, NULL, NULL, NULL, 0, &res);
    
    if (rc != LDAP_SUCCESS) {
        fprintf(stderr, "LDAP search failed: %s\n", ldap_err2string(rc));
    } else {
        if (ldap_count_entries(ld, res) == 0) {
            printf("User not found.\n");
        } else {
            LDAPMessage* entry;
            for (entry = ldap_first_entry(ld, res); entry != NULL; entry = ldap_next_entry(ld, entry)) {
                char* dn = ldap_get_dn(ld, entry);
                if (dn) {
                    printf("Found entry: %s\n", dn);
                    ldap_memfree(dn);
                }
            }
        }
    }

    if (res) ldap_msgfree(res);
    if (ld) ldap_unbind_ext_s(ld, NULL, NULL);
}

int main(int argc, char* argv[]) {
    // An LDAP server must be running on localhost:389 for this to work.
    if (argc > 1) {
        printf("--- Running with provided argument ---\n");
        findUserInLdap(argv[1]);
    } else {
        printf("--- Running with pre-defined test cases ---\n");
        printf("NOTE: These tests require a local LDAP server populated with sample data.\n");
        
        const char* testCases[] = {
            "jdoe@example.com",
            "admin@example.com",
            "test.user@sub.example.com",
            "*@example.com",
            "jdoe)(uid=*))(|(uid=*@example.com"
        };
        int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
        
        for (int i = 0; i < numTestCases; ++i) {
            printf("\n--- Test Case %d: %s ---\n", i + 1, testCases[i]);
            findUserInLdap(testCases[i]);
        }
    }
    return 0;
}