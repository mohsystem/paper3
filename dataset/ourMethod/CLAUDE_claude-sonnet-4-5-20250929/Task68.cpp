
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <ldap.h>
#include <cstring>
#include <memory>

// RAII wrapper for LDAP connection
class LDAPConnection {
private:
    LDAP* ld;
    
public:
    LDAPConnection() : ld(nullptr) {}
    
    ~LDAPConnection() {
        if (ld != nullptr) {
            ldap_unbind_ext_s(ld, nullptr, nullptr);
        }
    }
    
    LDAP* get() { return ld; }
    LDAP** getPtr() { return &ld; }
    
    // Prevent copying
    LDAPConnection(const LDAPConnection&) = delete;
    LDAPConnection& operator=(const LDAPConnection&) = delete;
};

// Validate domain component - only alphanumeric and hyphens, max 63 chars per component
bool validateDC(const std::string& dc) {
    if (dc.empty() || dc.length() > 253) { // Max DNS name length
        return false;
    }
    
    std::regex dcPattern("^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\\\\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$");
    return std::regex_match(dc, dcPattern);
}

// Validate username - alphanumeric, underscore, hyphen, dot, max 64 chars
bool validateUsername(const std::string& username) {
    if (username.empty() || username.length() > 64) {
        return false;
    }
    
    std::regex usernamePattern("^[a-zA-Z0-9._-]+$");
    return std::regex_match(username, usernamePattern);
}

// Escape LDAP special characters to prevent injection
std::string escapeLDAPString(const std::string& input) {
    std::string escaped;
    escaped.reserve(input.length() * 2); // Reserve space for potential escaping
    
    for (char c : input) {
        switch (c) {
            case '*':
            case '(':
            case ')':
            case '\\\\':\n            case '\\0':\n                escaped += '\\\\';\n                escaped += c;\n                break;\n            default:\n                escaped += c;\n        }\n    }\n    return escaped;\n}\n\n// Convert domain to LDAP base DN format (e.g., "example.com" -> "dc=example,dc=com")\nstd::string domainToBaseDN(const std::string& domain) {\n    std::string baseDN;\n    size_t start = 0;\n    size_t end = domain.find('.');\n    \n    while (end != std::string::npos) {\n        if (!baseDN.empty()) {\n            baseDN += ",";\n        }\n        baseDN += "dc=" + escapeLDAPString(domain.substr(start, end - start));\n        start = end + 1;\n        end = domain.find('.', start);
    }
    
    if (!baseDN.empty()) {
        baseDN += ",";
    }
    baseDN += "dc=" + escapeLDAPString(domain.substr(start));
    
    return baseDN;
}

// Search LDAP and return user information
std::string searchLDAP(const std::string& username, const std::string& dc) {
    // Validate inputs before processing
    if (!validateUsername(username)) {
        return "Error: Invalid username format";
    }
    
    if (!validateDC(dc)) {
        return "Error: Invalid domain component format";
    }
    
    LDAPConnection ldapConn;
    int result;
    
    // Initialize LDAP connection to localhost
    const char* ldapURI = "ldap://localhost:389";
    result = ldap_initialize(ldapConn.getPtr(), ldapURI);
    if (result != LDAP_SUCCESS) {
        return "Error: Failed to initialize LDAP connection";
    }
    
    // Set LDAP version to 3
    int version = LDAP_VERSION3;
    result = ldap_set_option(ldapConn.get(), LDAP_OPT_PROTOCOL_VERSION, &version);
    if (result != LDAP_SUCCESS) {
        return "Error: Failed to set LDAP protocol version";
    }
    
    // Set network timeout to prevent hanging
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    ldap_set_option(ldapConn.get(), LDAP_OPT_NETWORK_TIMEOUT, &timeout);
    
    // Anonymous bind (no credentials passed in clear text)
    result = ldap_simple_bind_s(ldapConn.get(), nullptr, nullptr);
    if (result != LDAP_SUCCESS) {
        return "Error: LDAP bind failed";
    }
    
    // Construct base DN from domain component
    std::string baseDN = domainToBaseDN(dc);
    
    // Construct LDAP filter with escaped username to prevent injection
    std::string filter = "(uid=" + escapeLDAPString(username) + ")";
    
    // Define attributes to retrieve (avoid retrieving sensitive attributes)
    const char* attrs[] = {"uid", "cn", "mail", "displayName", nullptr};
    
    // Perform LDAP search
    LDAPMessage* searchResult = nullptr;
    result = ldap_search_ext_s(
        ldapConn.get(),
        baseDN.c_str(),
        LDAP_SCOPE_SUBTREE,
        filter.c_str(),
        const_cast<char**>(attrs),
        0,
        nullptr,
        nullptr,
        &timeout,
        1000, // Size limit
        &searchResult
    );
    
    if (result != LDAP_SUCCESS) {
        if (searchResult) {
            ldap_msgfree(searchResult);
        }
        return "Error: LDAP search failed";
    }
    
    // Process search results
    std::string userInfo;
    int numEntries = ldap_count_entries(ldapConn.get(), searchResult);
    
    if (numEntries == 0) {
        ldap_msgfree(searchResult);
        return "No user found";
    }
    
    LDAPMessage* entry = ldap_first_entry(ldapConn.get(), searchResult);
    if (entry) {
        char* dn = ldap_get_dn(ldapConn.get(), entry);
        if (dn) {
            userInfo += "DN: " + std::string(dn) + "\\n";
            ldap_memfree(dn);
        }
        
        // Iterate through attributes
        BerElement* ber = nullptr;
        char* attr = ldap_first_attribute(ldapConn.get(), entry, &ber);
        
        while (attr) {
            berval** vals = ldap_get_values_len(ldapConn.get(), entry, attr);
            if (vals) {
                for (int i = 0; vals[i] != nullptr; i++) {
                    userInfo += std::string(attr) + ": " + 
                               std::string(vals[i]->bv_val, vals[i]->bv_len) + "\\n";
                }
                ldap_value_free_len(vals);
            }
            ldap_memfree(attr);
            attr = ldap_next_attribute(ldapConn.get(), entry, ber);
        }
        
        if (ber) {
            ber_free(ber, 0);
        }
    }
    
    ldap_msgfree(searchResult);
    return userInfo;
}

int main(int argc, char* argv[]) {
    // Test cases
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"testuser1", "example.com"},
        {"john.doe", "corp.example.com"},
        {"admin", "test.local"},
        {"user_123", "domain.org"},
        {"validuser", "sub.domain.net"}
    };
    
    // If command line arguments provided, use them; otherwise run test cases
    if (argc == 3) {
        std::string username = argv[1];
        std::string dc = argv[2];
        std::string result = searchLDAP(username, dc);
        std::cout << result << std::endl;
    } else {
        std::cout << "Running test cases..." << std::endl;
        for (const auto& testCase : testCases) {
            std::cout << "\\n--- Testing username: " << testCase.first 
                      << ", dc: " << testCase.second << " ---" << std::endl;
            std::string result = searchLDAP(testCase.first, testCase.second);
            std::cout << result << std::endl;
        }
    }
    
    return 0;
}
