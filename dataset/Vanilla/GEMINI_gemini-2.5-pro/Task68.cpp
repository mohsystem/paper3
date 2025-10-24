// Requires OpenLDAP development libraries.
// On Debian/Ubuntu: sudo apt-get install libldap2-dev
// On RedHat/CentOS: sudo yum install openldap-devel
// Compile with: g++ -o Task68 Task68.cpp -lldap -llber

#include <iostream>
#include <string>
#include <vector>
#include <ldap.h>

std::string searchLdapUser(const std::string& baseDn, const std::string& username) {
    LDAP* ld = nullptr;
    const char* ldap_host = "localhost";
    const int ldap_port = 389;
    std::string result_str;

    // Initialize LDAP session
    if (ldap_initialize(&ld, (std::string("ldap://") + ldap_host + ":" + std::to_string(ldap_port)).c_str()) != LDAP_SUCCESS) {
        return "Error: ldap_initialize failed.\n";
    }

    // Set LDAP protocol version to 3
    int ldap_version = LDAP_VERSION3;
    if (ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldap_version) != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "Error: ldap_set_option failed to set version 3.\n";
    }

    // Bind anonymously
    if (ldap_simple_bind_s(ld, nullptr, nullptr) != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "Error: ldap_simple_bind_s failed.\n";
    }

    // Construct search filter
    std::string filter = "(&(objectClass=person)(uid=" + username + "))";
    
    LDAPMessage* res = nullptr;
    int rc = ldap_search_ext_s(ld, baseDn.c_str(), LDAP_SCOPE_SUBTREE, filter.c_str(), nullptr, 0, nullptr, nullptr, nullptr, 0, &res);

    if (rc != LDAP_SUCCESS) {
        result_str = "LDAP Search Error: " + std::string(ldap_err2string(rc)) + "\n";
    } else {
        int num_entries = ldap_count_entries(ld, res);
        if (num_entries == 0) {
            result_str = "User '" + username + "' not found in '" + baseDn + "'.\n";
        } else {
            LDAPMessage* entry = ldap_first_entry(ld, res);
            while (entry != nullptr) {
                char* dn = ldap_get_dn(ld, entry);
                result_str += "Found entry: " + std::string(dn) + "\n";
                ldap_memfree(dn);

                BerElement* ber = nullptr;
                for (char* attr = ldap_first_attribute(ld, entry, &ber); attr != nullptr; attr = ldap_next_attribute(ld, entry, ber)) {
                    result_str += "  Attribute: " + std::string(attr) + "\n";
                    berval** vals = ldap_get_values_len(ld, entry, attr);
                    if (vals != nullptr) {
                        for (int i = 0; vals[i] != nullptr; ++i) {
                            result_str += "    Value: " + std::string(vals[i]->bv_val) + "\n";
                        }
                        ldap_value_free_len(vals);
                    }
                    ldap_memfree(attr);
                }
                ber_free(ber, 1);
                entry = ldap_next_entry(ld, entry);
            }
        }
    }

    if (res != nullptr) ldap_msgfree(res);
    ldap_unbind_ext_s(ld, nullptr, nullptr);
    return result_str;
}

int main(int argc, char* argv[]) {
    if (argc > 1 && (argc - 1) % 2 == 0) {
        std::cout << "Running test cases from command line arguments..." << std::endl;
        for (int i = 1; i < argc; i += 2) {
            std::string baseDn = argv[i];
            std::string username = argv[i+1];
            std::cout << "--- Test Case " << (i/2 + 1) << ": baseDn=" << baseDn << ", username=" << username << " ---" << std::endl;
            std::string result = searchLdapUser(baseDn, username);
            std::cout << result << std::endl;
        }
    } else {
        std::cout << "Running predefined test cases..." << std::endl;
        // Test Case 1: A standard valid user search
        std::cout << "--- Test Case 1: Find user 'jdoe' in 'dc=example,dc=com' ---" << std::endl;
        std::cout << searchLdapUser("dc=example,dc=com", "jdoe") << std::endl;

        // Test Case 2: Another valid user
        std::cout << "--- Test Case 2: Find user 'asmith' in 'dc=example,dc=com' ---" << std::endl;
        std::cout << searchLdapUser("dc=example,dc=com", "asmith") << std::endl;

        // Test Case 3: A user that does not exist
        std::cout << "--- Test Case 3: Find non-existent user 'nobody' ---" << std::endl;
        std::cout << searchLdapUser("dc=example,dc=com", "nobody") << std::endl;

        // Test Case 4: A search in a non-existent base DN
        std::cout << "--- Test Case 4: Search in a non-existent base DN ---" << std::endl;
        std::cout << searchLdapUser("dc=nonexistent,dc=org", "jdoe") << std::endl;

        // Test Case 5: A user in a different, valid base DN
        std::cout << "--- Test Case 5: Find user 'bcarter' in 'dc=test,dc=org' ---" << std::endl;
        std::cout << searchLdapUser("dc=test,dc=org", "bcarter") << std::endl;

        std::cout << "\nUsage for custom tests: ./Task68 <baseDn1> <user1> <baseDn2> <user2> ..." << std::endl;
        std::cout << "Example: ./Task68 dc=example,dc=com jdoe dc=example,dc=com nonexistant" << std::endl;
    }

    return 0;
}