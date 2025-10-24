/*
NOTE: Requires the OpenLDAP development library.
On Debian/Ubuntu: sudo apt-get install libldap2-dev
On RHEL/CentOS: sudo yum install openldap-devel

Compile with: g++ -o task68_cpp task68.cpp -lldap -llber

NOTE: This program requires a running LDAP server on localhost:389 with appropriate data.
*/

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <ldap.h>

// Helper function to escape special characters for use in an LDAP filter.
// Replaces '\', '*', '(', ')', and the NUL character with their hex-escaped representation.
// This is a crucial security measure to prevent LDAP injection.
std::string escapeLdapFilter(const std::string& input) {
    std::ostringstream escaped;
    for (char c : input) {
        switch (c) {
            case '\\': escaped << "\\5c"; break;
            case '*':  escaped << "\\2a"; break;
            case '(':  escaped << "\\28"; break;
            case ')':  escaped << "\\29"; break;
            case '\0': escaped << "\\00"; break;
            default:   escaped << c;
        }
    }
    return escaped.str();
}

// Main function to perform the LDAP search.
std::string searchLdapUser(const std::string& username, const std::string& domainComponent) {
    LDAP *ld = nullptr;
    LDAPMessage *res = nullptr;
    const char* ldap_uri = "ldap://localhost:389"; // For production, use "ldaps://..."
    int rc = 0;
    std::stringstream resultBuilder;

    // --- 1. Initialize LDAP session ---
    rc = ldap_initialize(&ld, ldap_uri);
    if (rc != LDAP_SUCCESS) {
        return "Error: ldap_initialize failed: " + std::string(ldap_err2string(rc));
    }

    int protocol_version = LDAP_VERSION3;
    rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &protocol_version);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "Error: Could not set LDAP version 3: " + std::string(ldap_err2string(rc));
    }

    // --- 2. Bind to the server (anonymous bind for this example) ---
    rc = ldap_simple_bind_s(ld, nullptr, nullptr);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "Error: Could not bind to LDAP server: " + std::string(ldap_err2string(rc));
    }

    // --- 3. Construct the search base from the domain component ---
    std::string searchBase = "dc=";
    std::string tempDc = domainComponent;
    size_t pos = 0;
    while ((pos = tempDc.find(',')) != std::string::npos) {
        searchBase += tempDc.substr(0, pos) + ",dc=";
        tempDc.erase(0, pos + 1);
    }
    searchBase += tempDc;
    
    // --- 4. Construct search filter with properly escaped username ---
    std::string escapedUsername = escapeLdapFilter(username);
    std::string searchFilter = "(uid=" + escapedUsername + ")";
    
    // Specify attributes to return to avoid exposing sensitive data
    char* attrs[] = { (char*)"cn", (char*)"sn", (char*)"mail", (char*)"uid", nullptr };

    // --- 5. Perform the synchronous search ---
    rc = ldap_search_ext_s(ld, searchBase.c_str(), LDAP_SCOPE_SUBTREE, searchFilter.c_str(), attrs, 0, nullptr, nullptr, nullptr, 0, &res);

    if (rc != LDAP_SUCCESS && rc != LDAP_NO_SUCH_OBJECT) {
        resultBuilder << "LDAP search error: " << ldap_err2string(rc) << "\n";
    }

    if (ldap_count_entries(ld, res) == 0) {
        resultBuilder << "User '" << username << "' not found in '" << searchBase << "'.\n";
    } else {
        resultBuilder << "Search results for user '" << username << "':\n";
        for (LDAPMessage* entry = ldap_first_entry(ld, res); entry != nullptr; entry = ldap_next_entry(ld, entry)) {
            char* dn = ldap_get_dn(ld, entry);
            if (dn) {
                resultBuilder << ">> DN: " << dn << "\n";
                ldap_memfree(dn);
            }

            BerElement* ber = nullptr;
            for (char* attr = ldap_first_attribute(ld, entry, &ber); attr != nullptr; attr = ldap_next_attribute(ld, entry, ber)) {
                berval** vals = ldap_get_values_len(ld, entry, attr);
                if (vals) {
                    resultBuilder << "   " << attr << ": ";
                    for (int i = 0; vals[i] != nullptr; i++) {
                        resultBuilder << vals[i]->bv_val << " ";
                    }
                    resultBuilder << "\n";
                    ldap_value_free_len(vals);
                }
                ldap_memfree(attr);
            }
            if(ber) ber_free(ber, 0);
        }
    }
    
    // --- 6. Clean up resources ---
    if (res) ldap_msgfree(res);
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return resultBuilder.str();
}

int main(int argc, char* argv[]) {
    if (argc == 3) {
        std::cout << "--- Running with Command Line Arguments ---" << std::endl;
        std::cout << searchLdapUser(argv[1], argv[2]);
    } else {
        std::cout << "Usage: " << argv[0] << " <username> <domain_component_string>" << std::endl;
        std::cout << "Example: " << argv[0] << " jdoe example,com" << std::endl << std::endl;
        std::cout << "NOTE: This program requires a running LDAP server on localhost:389 with appropriate data.\n" << std::endl;
        
        // --- 5 Hardcoded Test Cases ---
        std::cout << "--- Running 5 Hardcoded Test Cases ---" << std::endl;

        // Test Case 1: A valid user that should exist on the test server.
        std::cout << "--- Test Case 1: Valid User ---" << std::endl;
        std::cout << searchLdapUser("jdoe", "example,com") << std::endl;

        // Test Case 2: A user that should not exist.
        std::cout << "--- Test Case 2: Non-existent User ---" << std::endl;
        std::cout << searchLdapUser("nonexistentuser", "example,com") << std::endl;

        // Test Case 3: A user in a different domain component.
        std::cout << "--- Test Case 3: User in a different DC ---" << std::endl;
        std::cout << searchLdapUser("asmith", "test,org") << std::endl;
        
        // Test Case 4: An attempt at LDAP injection. The escaping should handle this safely.
        std::cout << "--- Test Case 4: Potential Injection Attempt ---" << std::endl;
        std::cout << searchLdapUser("jdoe)(uid=*", "example,com") << std::endl;
        
        // Test Case 5: A search for a user with special characters that would break an unescaped filter.
        std::cout << "--- Test Case 5: User with special chars ---" << std::endl;
        std::cout << searchLdapUser("user*with(special)chars", "example,com") << std::endl;
    }
    return 0;
}