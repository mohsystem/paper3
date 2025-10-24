#include <iostream>
#include <string>
#include <vector>
#include <regex>

// Note: This program requires the OpenLDAP client libraries.
// On Debian/Ubuntu: sudo apt-get install libldap2-dev
// On RedHat/CentOS: sudo yum install openldap-devel
// Compile with: g++ task.cpp -o task -lldap
#include <ldap.h>

/**
 * @brief Escapes special characters in a string for use in an LDAP filter
 * according to RFC 4515. This is crucial to prevent LDAP injection.
 * @param input The raw string to escape.
 * @return The escaped string safe for use in an LDAP filter.
 */
std::string escapeLdapFilter(const std::string& input) {
    std::string escaped;
    escaped.reserve(input.length());
    for (char c : input) {
        switch (c) {
            case '*':  escaped += "\\2a"; break;
            case '(':  escaped += "\\28"; break;
            case ')':  escaped += "\\29"; break;
            case '\\': escaped += "\\5c"; break;
            case '\0': escaped += "\\00"; break;
            default:   escaped += c;
        }
    }
    return escaped;
}

/**
 * @brief Extracts user info from an LDAP directory based on user input.
 * @param userInput A string in the format "username@domain.com".
 * @return A string containing the search results or an error message.
 */
std::string findUserInLdap(const std::string& userInput) {
    if (userInput.empty()) {
        return "Error: Input cannot be empty.";
    }

    // 1. Extract username and domain
    size_t atPos = userInput.find('@');
    if (atPos == std::string::npos || atPos == 0 || atPos == userInput.length() - 1) {
        return "Error: Invalid input format. Expected 'username@domain.com'.";
    }
    std::string username = userInput.substr(0, atPos);
    std::string domain = userInput.substr(atPos + 1);

    // 2. Validate inputs
    if (!std::regex_match(domain, std::regex("^[a-zA-Z0-9.-]+$"))) {
        return "Error: Domain contains invalid characters.";
    }
    if (!std::regex_match(username, std::regex("^[a-zA-Z0-9_.-]+$"))) {
        return "Error: Username contains invalid characters.";
    }

    // 3. Construct Base DN
    std::string baseDn = "dc=" + std::regex_replace(domain, std::regex("\\."), ",dc=");

    // 4. Sanitize username and construct filter
    std::string escapedUsername = escapeLdapFilter(username);
    std::string searchFilter = "(uid=" + escapedUsername + ")";

    LDAP* ld = nullptr;
    LDAPMessage* res = nullptr;
    const char* ldapUri = "ldap://localhost:389";
    std::string result_output;

    if (ldap_initialize(&ld, ldapUri) != LDAP_SUCCESS) {
        return "Error: ldap_initialize failed.";
    }

    int ldap_version = LDAP_VERSION3;
    if (ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldap_version) != LDAP_SUCCESS) {
        ldap_destroy(ld);
        return "Error: ldap_set_option failed.";
    }
    
    // Anonymous bind
    int rc = ldap_simple_bind_s(ld, NULL, NULL);
    if (rc != LDAP_SUCCESS) {
        ldap_destroy(ld);
        return "Error: ldap_simple_bind_s failed: " + std::string(ldap_err2string(rc));
    }

    result_output += "Searching for user '" + username + "' in '" + baseDn + "'\n";
    result_output += "Using filter: " + searchFilter + "\n";
    
    rc = ldap_search_ext_s(ld, baseDn.c_str(), LDAP_SCOPE_SUBTREE, searchFilter.c_str(), NULL, 0, NULL, NULL, NULL, 0, &res);

    if (rc != LDAP_SUCCESS) {
        result_output += "LDAP search failed: " + std::string(ldap_err2string(rc)) + "\n";
    } else {
        if (ldap_count_entries(ld, res) == 0) {
            result_output += "User not found.\n";
        } else {
            for (LDAPMessage* entry = ldap_first_entry(ld, res); entry != nullptr; entry = ldap_next_entry(ld, entry)) {
                char* dn = ldap_get_dn(ld, entry);
                if (dn) {
                    result_output += "Found entry: " + std::string(dn) + "\n";
                    ldap_memfree(dn);
                }
                
                BerElement* ber = nullptr;
                for (char* attr = ldap_first_attribute(ld, entry, &ber); attr != nullptr; attr = ldap_next_attribute(ld, entry, ber)) {
                    result_output += "  " + std::string(attr) + ": ";
                    berval** vals = ldap_get_values_len(ld, entry, attr);
                    if (vals != nullptr) {
                        for (int i = 0; vals[i] != nullptr; i++) {
                            result_output += std::string(vals[i]->bv_val, vals[i]->bv_len);
                            if (vals[i+1] != nullptr) result_output += ", ";
                        }
                        ldap_value_free_len(vals);
                    }
                    result_output += "\n";
                    ldap_memfree(attr);
                }
                if (ber) ber_free(ber, 1);
            }
        }
    }

    if (res) ldap_msgfree(res);
    if (ld) ldap_unbind_ext_s(ld, nullptr, nullptr);

    return result_output;
}

int main(int argc, char* argv[]) {
    // An LDAP server must be running on localhost:389 for this to work.
    if (argc > 1) {
        std::cout << "--- Running with provided argument ---" << std::endl;
        std::string result = findUserInLdap(argv[1]);
        std::cout << result << std::endl;
    } else {
        std::cout << "--- Running with pre-defined test cases ---" << std::endl;
        std::cout << "NOTE: These tests require a local LDAP server populated with sample data." << std::endl;

        std::vector<std::string> testCases = {
            "jdoe@example.com",
            "admin@example.com",
            "test.user@sub.example.com",
            "*@example.com",
            "jdoe)(uid=*))(|(uid=*@example.com"
        };
        
        for (size_t i = 0; i < testCases.size(); ++i) {
            std::cout << "\n--- Test Case " << i + 1 << ": " << testCases[i] << " ---" << std::endl;
            std::string result = findUserInLdap(testCases[i]);
            std::cout << result;
        }
    }
    return 0;
}