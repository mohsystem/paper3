
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
    #include <winldap.h>
    #pragma comment(lib, "wldap32.lib")
#else
    #include <ldap.h>
#endif

using namespace std;

string searchLdapUser(const string& domainComponent, const string& username) {
    stringstream result;
    LDAP* ldap = nullptr;
    
    try {
        // Initialize LDAP connection
        #ifdef _WIN32
        ldap = ldap_init((PCHAR)"localhost", 389);
        #else
        int rc = ldap_initialize(&ldap, "ldap://localhost:389");
        if (rc != LDAP_SUCCESS) {
            result << "Error: Failed to initialize LDAP connection";
            return result.str();
        }
        #endif
        
        if (ldap == nullptr) {
            result << "Error: Failed to initialize LDAP connection";
            return result.str();
        }
        
        // Set LDAP version to 3
        int version = LDAP_VERSION3;
        ldap_set_option(ldap, LDAP_OPT_PROTOCOL_VERSION, &version);
        
        // Bind anonymously
        int bindResult = ldap_simple_bind_s(ldap, nullptr, nullptr);
        if (bindResult != LDAP_SUCCESS) {
            result << "Error: Failed to bind to LDAP server - " << ldap_err2string(bindResult);
            ldap_unbind_s(ldap);
            return result.str();
        }
        
        // Construct base DN
        string baseDN = "dc=" + domainComponent;
        size_t pos = 0;
        while ((pos = baseDN.find(".", pos)) != string::npos) {
            baseDN.replace(pos, 1, ",dc=");
            pos += 4;
        }
        
        // Construct search filter
        string searchFilter = "(uid=" + username + ")";
        
        // Perform search
        LDAPMessage* searchResult = nullptr;
        int searchRC = ldap_search_s(ldap, (char*)baseDN.c_str(), LDAP_SCOPE_SUBTREE,
                                     (char*)searchFilter.c_str(), nullptr, 0, &searchResult);
        
        if (searchRC == LDAP_SUCCESS && searchResult != nullptr) {
            int count = ldap_count_entries(ldap, searchResult);
            
            if (count > 0) {
                LDAPMessage* entry = ldap_first_entry(ldap, searchResult);
                char* dn = ldap_get_dn(ldap, entry);
                result << "User found: " << dn << "\\n";
                result << "Attributes:\\n";
                ldap_memfree(dn);
                
                BerElement* ber = nullptr;
                char* attr = ldap_first_attribute(ldap, entry, &ber);
                while (attr != nullptr) {
                    char** vals = ldap_get_values(ldap, entry, attr);
                    if (vals != nullptr) {
                        result << "  " << attr << ": " << vals[0] << "\\n";
                        ldap_value_free(vals);
                    }
                    ldap_memfree(attr);
                    attr = ldap_next_attribute(ldap, entry, ber);
                }
                if (ber != nullptr) ber_free(ber, 0);
            } else {
                result << "User not found";
            }
            
            ldap_msgfree(searchResult);
        } else {
            result << "Error: Search failed - " << ldap_err2string(searchRC);
        }
        
        ldap_unbind_s(ldap);
        
    } catch (...) {
        result << "Error: Exception occurred";
        if (ldap != nullptr) ldap_unbind_s(ldap);
    }
    
    return result.str();
}

int main(int argc, char* argv[]) {
    if (argc >= 3) {
        string domainComponent = argv[1];
        string username = argv[2];
        cout << searchLdapUser(domainComponent, username) << endl;
    } else {
        // Test cases
        cout << "Test Case 1:" << endl;
        cout << searchLdapUser("example.com", "john.doe") << endl;
        cout << "\\n---\\n" << endl;
        
        cout << "Test Case 2:" << endl;
        cout << searchLdapUser("company.org", "alice") << endl;
        cout << "\\n---\\n" << endl;
        
        cout << "Test Case 3:" << endl;
        cout << searchLdapUser("test.local", "bob.smith") << endl;
        cout << "\\n---\\n" << endl;
        
        cout << "Test Case 4:" << endl;
        cout << searchLdapUser("domain.net", "admin") << endl;
        cout << "\\n---\\n" << endl;
        
        cout << "Test Case 5:" << endl;
        cout << searchLdapUser("mycompany.com", "testuser") << endl;
    }
    
    return 0;
}
