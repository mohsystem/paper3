
// C++ does not have native JNDI support as it's a Java-specific API\n// This is a simulation of the naming service concept\n\n#include <iostream>\n#include <string>\n#include <map>\n#include <memory>\n#include <regex>\n#include <stdexcept>\n\nclass SecurityException : public std::runtime_error {\npublic:\n    explicit SecurityException(const std::string& message) \n        : std::runtime_error(message) {}\n};\n\nclass Task107 {\nprivate:\n    std::map<std::string, std::shared_ptr<void>> localRegistry;\n    \n    bool isDangerousProtocol(const std::string& jndiName) {\n        std::regex ldapPattern("^ldap://", std::regex::icase);\n        std::regex rmiPattern("^rmi://", std::regex::icase);\n        std::regex dnsPattern("^dns://", std::regex::icase);\n        std::regex iiopPattern("^iiop://", std::regex::icase);\n        \n        return std::regex_search(jndiName, ldapPattern) ||\n               std::regex_search(jndiName, rmiPattern) ||\n               std::regex_search(jndiName, dnsPattern) ||\n               std::regex_search(jndiName, iiopPattern);\n    }\n    \npublic:\n    // Secure JNDI lookup simulation\n    std::shared_ptr<void> performJNDILookup(const std::string& jndiName, \n                                           const std::string& contextFactory = "", \n                                           const std::string& providerUrl = "") {\n        // Input validation\n        if (jndiName.empty() || jndiName.find_first_not_of(' ') == std::string::npos) {
            throw std::invalid_argument("JNDI name cannot be null or empty");
        }
        
        // Whitelist validation - block remote protocols
        if (isDangerousProtocol(jndiName)) {
            throw SecurityException("Remote JNDI lookups are not allowed for security reasons");
        }
        
        // Only allow java: namespace
        if (jndiName.substr(0, 5) != "java:") {
            throw SecurityException("Only java: namespace is allowed");
        }
        
        try {
            auto it = localRegistry.find(jndiName);
            if (it != localRegistry.end()) {
                std::cout << "Successfully retrieved resource: " << jndiName << std::endl;
                return it->second;
            } else {
                std::cout << "Resource not found: " << jndiName << std::endl;
                return nullptr;
            }
        } catch (const std::exception& e) {
            std::cerr << "JNDI lookup failed for: " << jndiName << std::endl;
            std::cerr << "Error: " << e.what() << std::endl;
            return nullptr;
        }
    }
    
    std::shared_ptr<void> performSecureLocalLookup(const std::string& jndiName) {
        return performJNDILookup(jndiName);
    }
    
    void bindResource(const std::string& jndiName, std::shared_ptr<void> resource) {
        if (jndiName.empty() || jndiName.substr(0, 5) != "java:") {
            throw SecurityException("Can only bind to java: namespace");
        }
        
        try {
            localRegistry[jndiName] = resource;
            std::cout << "Successfully bound resource: " << jndiName << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Failed to bind resource: " << e.what() << std::endl;
        }
    }
};

int main() {
    std::cout << "=== JNDI Lookup Security Demo (C++ Simulation) ===" << std::endl << std::endl;
    
    Task107 task;
    
    // Bind a test resource
    auto testResource = std::make_shared<std::string>("TestDatabase");
    task.bindResource("java:comp/env/example", testResource);
    std::cout << std::endl;
    
    // Test Case 1: Valid local lookup
    std::cout << "Test Case 1: Valid local java: namespace lookup" << std::endl;
    try {
        auto result1 = task.performSecureLocalLookup("java:comp/env/example");
        std::cout << "Result: " << (result1 ? "Found" : "Not found") << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Expected behavior: " << e.what() << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 2: Attempting LDAP lookup (should be blocked)
    std::cout << "Test Case 2: Blocked LDAP lookup attempt" << std::endl;
    try {
        auto result2 = task.performJNDILookup("ldap://malicious.com/exploit");
        std::cout << "Result: " << result2 << std::endl;
    } catch (const SecurityException& e) {
        std::cout << "Security blocked: " << e.what() << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 3: Attempting RMI lookup (should be blocked)
    std::cout << "Test Case 3: Blocked RMI lookup attempt" << std::endl;
    try {
        auto result3 = task.performJNDILookup("rmi://malicious.com/exploit");
        std::cout << "Result: " << result3 << std::endl;
    } catch (const SecurityException& e) {
        std::cout << "Security blocked: " << e.what() << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 4: Empty JNDI name
    std::cout << "Test Case 4: Empty JNDI name validation" << std::endl;
    try {
        auto result4 = task.performJNDILookup("");
        std::cout << "Result: " << result4 << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cout << "Validation error: " << e.what() << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 5: Invalid namespace
    std::cout << "Test Case 5: Invalid namespace attempt" << std::endl;
    try {
        auto result5 = task.performJNDILookup("file:///etc/passwd");
        std::cout << "Result: " << result5 << std::endl;
    } catch (const SecurityException& e) {
        std::cout << "Security blocked: " << e.what() << std::endl;
    }
    std::cout << std::endl;
    
    std::cout << "=== Security Summary ===" << std::endl;
    std::cout << "✓ Remote lookups (LDAP, RMI, DNS, IIOP) are blocked" << std::endl;
    std::cout << "✓ Only local java: namespace is allowed" << std::endl;
    std::cout << "✓ Input validation prevents injection attacks" << std::endl;
    std::cout << "✓ Proper error handling and resource cleanup" << std::endl;
    
    return 0;
}
