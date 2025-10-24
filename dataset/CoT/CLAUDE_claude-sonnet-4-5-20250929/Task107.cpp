
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <algorithm>

class Task107 {
private:
    static const std::regex SAFE_JNDI_PATTERN;
    static const std::vector<std::string> ALLOWED_CONTEXTS;
    
public:
    // Performs secure JNDI lookup with validation
    static void* performSecureJNDILookup(const std::string& jndiName) {
        if (jndiName.empty()) {
            std::cout << "Error: JNDI name cannot be null or empty" << std::endl;
            return nullptr;
        }
        
        // Validate JNDI name format
        if (!std::regex_match(jndiName, SAFE_JNDI_PATTERN)) {
            std::cout << "Error: Invalid JNDI name format: " << jndiName << std::endl;
            return nullptr;
        }
        
        // Check if JNDI name starts with allowed context
        bool isAllowed = false;
        for (const auto& allowedContext : ALLOWED_CONTEXTS) {
            if (jndiName.find(allowedContext) == 0) {
                isAllowed = true;
                break;
            }
        }
        
        if (!isAllowed) {
            std::cout << "Error: JNDI context not allowed: " << jndiName << std::endl;
            return nullptr;
        }
        
        // Simulate JNDI lookup (C++ doesn't have native JNDI)
        std::cout << "Successfully validated JNDI name: " << jndiName << std::endl;
        return reinterpret_cast<void*>(1); // Simulated resource pointer
    }
    
    // Validates if a JNDI name is safe and allowed
    static bool isValidJNDIName(const std::string& jndiName) {
        if (jndiName.empty()) {
            return false;
        }
        
        if (!std::regex_match(jndiName, SAFE_JNDI_PATTERN)) {
            return false;
        }
        
        for (const auto& allowedContext : ALLOWED_CONTEXTS) {
            if (jndiName.find(allowedContext) == 0) {
                return true;
            }
        }
        
        return false;
    }
};

const std::regex Task107::SAFE_JNDI_PATTERN("^[a-zA-Z0-9/_:-]+$");
const std::vector<std::string> Task107::ALLOWED_CONTEXTS = {
    "java:comp/env/jdbc/",
    "java:comp/env/jms/",
    "java:comp/env/mail/"
};

int main() {
    std::cout << "=== Secure JNDI Lookup Program ===" << std::endl << std::endl;
    
    // Test Case 1: Valid JDBC lookup
    std::cout << "Test 1: Valid JDBC lookup" << std::endl;
    Task107::performSecureJNDILookup("java:comp/env/jdbc/myDataSource");
    
    // Test Case 2: Valid JMS lookup
    std::cout << "\\nTest 2: Valid JMS lookup" << std::endl;
    Task107::performSecureJNDILookup("java:comp/env/jms/myQueue");
    
    // Test Case 3: Invalid characters (injection attempt)
    std::cout << "\\nTest 3: Invalid characters (security test)" << std::endl;
    Task107::performSecureJNDILookup("java:comp/env/jdbc/../../../malicious");
    
    // Test Case 4: Unauthorized context
    std::cout << "\\nTest 4: Unauthorized context" << std::endl;
    Task107::performSecureJNDILookup("rmi://malicious.server/Exploit");
    
    // Test Case 5: Empty JNDI name
    std::cout << "\\nTest 5: Empty JNDI name" << std::endl;
    Task107::performSecureJNDILookup("");
    
    // Test validation function
    std::cout << "\\n=== JNDI Name Validation Tests ===" << std::endl;
    std::cout << "Valid name: " << Task107::isValidJNDIName("java:comp/env/jdbc/testDB") << std::endl;
    std::cout << "Invalid name: " << Task107::isValidJNDIName("ldap://evil.com/exploit") << std::endl;
    
    return 0;
}
