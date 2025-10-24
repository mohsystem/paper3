
#include <iostream>
#include <string>
#include <ldap.h>
#include <cstring>

using namespace std;

string searchLDAP(const string& dcComponent, const string& username) {
    string result = "";
    LDAP* ldap = nullptr;
    
    try {
        // Initialize LDAP connection
        int rc = ldap_initialize(&ldap, "ldap://localhost:389");
        if (rc != LDAP_SUCCESS) {
            return "Error: Failed to initialize LDAP connection";
        }
        
        // Set LDAP version to 3
        int version = LDAP_VERSION3;
        ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &version);
        
        // Anonymous bind
        rc = ldap_simple_bind_s(ldap, nullptr, nullptr);
        if (rc != LDAP_SUCCESS) {
            ldap_unbind_ext_s(ldap, nullptr, nullptr);
            return "Error: Failed to bind to LDAP server";
        }
        
        // Construct base DN from DC component
        string baseDN = "dc=";
        size_t pos = 0;
        string dc = dcComponent;
        while ((pos = dc.find('.')) != string::npos) {
            baseDN += dc.substr(0, pos) + ",dc=";
            dc.erase(0, pos + 1);
        }
        baseDN += dc;
        
        // Construct LDAP search filter (VULNERABLE TO INJECTION)
        string searchFilter = "(uid=" + username + ")";
        
        // Perform search
        LDAPMessage* searchResult = nullptr;
        rc = ldap_search_ext_s(
            ldap,
            baseDN.c_str(),
            LDAP_SCOPE_SUBTREE,
            searchFilter.c_str(),
            nullptr,
            0,
            nullptr,
            nullptr,
            nullptr,
            0,
            &searchResult
        );
        
        if (rc == LDAP_SUCCESS && searchResult) {
            // Process results
            LDAPMessage* entry = ldap_first_entry(ldap, searchResult);
            
            if (entry) {
                while (entry) {
                    char* dn = ldap_get_dn(ldap, entry);
                    if (dn) {
                        result += "DN: " + string(dn) + "\\n";
                        ldap_memfree(dn);
                    }
                    
                    BerElement* ber = nullptr;
                    char* attr = ldap_first_attribute(ldap, entry, &ber);
                    
                    while (attr) {
                        struct berval** vals = ldap_get_values_len(ldap, entry, attr);
                        if (vals) {
                            for (int i = 0; vals[i] != nullptr; i++) {
                                result += string(attr) + ": " + string(vals[i]->bv_val, vals[i]->bv_len) + "\\n";
                            }
                            ldap_value_free_len(vals);
                        }
                        ldap_memfree(attr);
                        attr = ldap_next_attribute(ldap, entry, ber);
                    }
                    
                    if (ber) ber_free(ber, 0);
                    result += "\\n";
                    entry = ldap_next_entry(ldap, entry);
                }
            } else {
                result = "No user found";
            }
            
            ldap_msgfree(searchResult);
        } else {
            result = "Error: Search failed - " + string(ldap_err2string(rc));
        }
        
        ldap_unbind_ext_s(ldap, nullptr, nullptr);
        
    } catch (...) {
        if (ldap) ldap_unbind_ext_s(ldap, nullptr, nullptr);
        result = "Error: Exception occurred";
    }
    
    return result;
}

int main(int argc, char* argv[]) {
    // Test cases
    cout << "Test Case 1:" << endl;
    cout << searchLDAP("example.com", "john.doe") << endl;
    cout << "---" << endl;
    
    cout << "Test Case 2:" << endl;
    cout << searchLDAP("company.org", "admin") << endl;
    cout << "---" << endl;
    
    cout << "Test Case 3:" << endl;
    cout << searchLDAP("test.local", "user123") << endl;
    cout << "---" << endl;
    
    cout << "Test Case 4:" << endl;
    cout << searchLDAP("domain.net", "testuser") << endl;
    cout << "---" << endl;
    
    cout << "Test Case 5:" << endl;
    cout << searchLDAP("myorg.com", "employee") << endl;
    cout << "---" << endl;
    
    // Command line argument handling
    if (argc >= 3) {
        cout << "\\nCommand Line Arguments:" << endl;
        cout << searchLDAP(argv[1], argv[2]) << endl;
    }
    
    return 0;
}
