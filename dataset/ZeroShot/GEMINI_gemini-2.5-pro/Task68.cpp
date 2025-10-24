#include <iostream>
#include <string>
#include <vector>
#include <ldap.h>
#include <sstream>

// NOTE: This code requires the OpenLDAP client development libraries.
// On Debian/Ubuntu: sudo apt-get install libldap2-dev
// On RHEL/CentOS: sudo yum install openldap-devel
// To compile: g++ -o Task68 Task68.cpp -lldap

std::string searchLdapUser(const std::string& dcString, const std::string& username) {
    LDAP* ld = nullptr;
    const char* ldap_host = "localhost";
    const int ldap_port = 389;
    int rc = 0;
    std::stringstream resultBuilder;
    std::string ldap_uri = "ldap://" + std::string(ldap_host) + ":" + std::to_string(ldap_port);

    rc = ldap_initialize(&ld, ldap_uri.c_str());
    if (rc != LDAP_SUCCESS) {
        return "ldap_initialize failed: " + std::string(ldap_err2string(rc));
    }

    int protocol_version = LDAP_VERSION3;
    rc = ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &protocol_version);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "ldap_set_option failed: " + std::string(ldap_err2string(rc));
    }

    rc = ldap_simple_bind_s(ld, nullptr, nullptr);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "ldap_simple_bind_s failed: " + std::string(ldap_err2string(rc));
    }

    std::string searchBase = "dc=" + dcString;
    size_t pos = 0;
    while ((pos = searchBase.find(',', pos)) != std::string::npos) {
        searchBase.replace(pos, 1, ",dc=");
        pos += 4;
    }

    // Securely construct the search filter using ldap_bv2escaped_filter_value_x
    BerValue berUsername;
    berUsername.bv_val = (char*)username.c_str();
    berUsername.bv_len = username.length();
    BerValue* escaped_val;
    std::string searchFilter;

    rc = ldap_bv2escaped_filter_value_x(ld, &berUsername, &escaped_val, 0);
    if (rc != LDAP_SUCCESS) {
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return "Failed to escape filter value";
    }
    searchFilter = "(cn=" + std::string(escaped_val->bv_val, escaped_val->bv_len) + ")";
    ldap_memfree(escaped_val->bv_val);
    ldap_memfree(escaped_val);

    resultBuilder << "Searching for user '" << username << "'\n";
    resultBuilder << "Base DN: " << searchBase << "\n";
    resultBuilder << "Filter: " << searchFilter << "\n\n";

    LDAPMessage* res = nullptr;
    rc = ldap_search_ext_s(ld, searchBase.c_str(), LDAP_SCOPE_SUBTREE, searchFilter.c_str(), nullptr, 0, nullptr, nullptr, nullptr, 0, &res);

    if (rc != LDAP_SUCCESS && rc != LDAP_SIZELIMIT_EXCEEDED) {
        resultBuilder << "ldap_search_ext_s failed: " << ldap_err2string(rc) << "\n";
    } else {
        int num_entries = ldap_count_entries(ld, res);
        if (num_entries == 0) {
            resultBuilder << "User '" << username << "' not found.\n";
        } else {
            LDAPMessage* entry = ldap_first_entry(ld, res);
            while (entry != nullptr) {
                char* dn = ldap_get_dn(ld, entry);
                if (dn) {
                    resultBuilder << "Found user DN: " << dn << "\n";
                    ldap_memfree(dn);
                }
                BerElement* ber = nullptr;
                for (char* attr = ldap_first_attribute(ld, entry, &ber); attr != nullptr; attr = ldap_next_attribute(ld, entry, ber)) {
                    berval** vals = ldap_get_values_len(ld, entry, attr);
                    if (vals != nullptr) {
                        for (int i = 0; vals[i] != nullptr; i++) {
                            resultBuilder << attr << ": " << vals[i]->bv_val << "\n";
                        }
                        ldap_value_free_len(vals);
                    }
                    ldap_memfree(attr);
                }
                if (ber != nullptr) { ber_free(ber, 1); }
                entry = ldap_next_entry(ld, entry);
            }
        }
    }
    
    if (res != nullptr) ldap_msgfree(res);
    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return resultBuilder.str();
}

void run_test(int test_num, const std::string& username, const std::string& dcString) {
    std::cout << "----------- Test Case " << test_num << " -----------" << std::endl;
    std::string result = searchLdapUser(dcString, username);
    std::cout << result << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "NOTE: This program requires the OpenLDAP client development libraries." << std::endl;
    std::cout << "NOTE: This program also requires an LDAP server running on localhost:389." << std::endl;
    std::cout << "For tests to succeed, it should have a base DN like 'dc=example,dc=com'" << std::endl;
    std::cout << "and users with cn='jdoe', 'test(user)', 'star*user', etc.\n" << std::endl;

    run_test(1, "jdoe", "example,com");
    run_test(2, "nonexistent", "example,com");
    run_test(3, "test(user)", "example,com");
    run_test(4, "star*user", "example,com");
    run_test(5, "*", "example,com");

    return 0;
}