
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <stdexcept>
#include <regex>
#include <cstring>

// Simulated JNDI context for CPP (Java-style JNDI doesn't exist in C++)
// This implementation provides a safe, controlled registry pattern
// Security: No dynamic code loading, no reflection, strict validation

class Resource {
public:
    virtual ~Resource() = default;
    virtual std::string getType() const = 0;
    virtual std::string getValue() const = 0;
};

class StringResource : public Resource {
private:
    std::string value;
public:
    explicit StringResource(const std::string& val) : value(val) {}
    std::string getType() const override { return "String"; }
    std::string getValue() const override { return value; }
};

class DataSourceResource : public Resource {
private:
    std::string connectionString;
public:
    explicit DataSourceResource(const std::string& conn) : connectionString(conn) {}
    std::string getType() const override { return "DataSource"; }
    std::string getValue() const override { return connectionString; }
};

class SecureJNDIContext {
private:
    std::map<std::string, std::shared_ptr<Resource>> registry;
    
    // Security: Validate JNDI name to prevent injection attacks
    // Only allow alphanumeric, underscore, forward slash, and dot
    bool isValidName(const std::string& name) const {
        if (name.empty() || name.length() > 256) {
            return false;
        }
        // Whitelist validation: only safe characters
        std::regex validPattern("^[a-zA-Z0-9_/.]+$");
        return std::regex_match(name, validPattern);
    }
    
    // Security: Prevent path traversal attacks
    bool containsPathTraversal(const std::string& name) const {
        return name.find("..") != std::string::npos ||
               name.find("://") != std::string::npos;
    }
    
public:
    // Security: No deserialization, no remote lookups, controlled registry only
    void bind(const std::string& name, std::shared_ptr<Resource> resource) {
        // Input validation: check name format
        if (!isValidName(name)) {
            throw std::invalid_argument("Invalid JNDI name format");
        }
        
        // Security: prevent path traversal
        if (containsPathTraversal(name)) {
            throw std::invalid_argument("Path traversal detected in JNDI name");
        }
        
        if (!resource) {
            throw std::invalid_argument("Resource cannot be null");
        }
        
        registry[name] = resource;
    }
    
    // Security: Safe lookup with validation, no remote access
    std::shared_ptr<Resource> lookup(const std::string& name) {
        // Input validation
        if (!isValidName(name)) {
            throw std::invalid_argument("Invalid JNDI name format");
        }
        
        // Security: prevent path traversal
        if (containsPathTraversal(name)) {
            throw std::invalid_argument("Path traversal detected in JNDI name");
        }
        
        auto it = registry.find(name);
        if (it == registry.end()) {
            throw std::runtime_error("Resource not found: " + name);
        }
        
        return it->second;
    }
    
    void unbind(const std::string& name) {
        if (!isValidName(name)) {
            throw std::invalid_argument("Invalid JNDI name format");
        }
        registry.erase(name);
    }
};

// Test function
int main() {
    try {
        SecureJNDIContext context;
        
        // Test case 1: Bind and lookup string resource
        std::cout << "Test 1: String resource lookup" << std::endl;
        auto stringRes = std::make_shared<StringResource>("Hello JNDI");
        context.bind("app/config/message", stringRes);
        auto result1 = context.lookup("app/config/message");
        std::cout << "Type: " << result1->getType() << ", Value: " << result1->getValue() << std::endl;
        
        // Test case 2: Bind and lookup datasource
        std::cout << "\\nTest 2: DataSource resource lookup" << std::endl;
        auto dsRes = std::make_shared<DataSourceResource>("jdbc:mysql://localhost:3306/testdb");
        context.bind("jdbc/MyDataSource", dsRes);
        auto result2 = context.lookup("jdbc/MyDataSource");
        std::cout << "Type: " << result2->getType() << ", Value: " << result2->getValue() << std::endl;
        
        // Test case 3: Invalid name with special characters (should fail)
        std::cout << "\\nTest 3: Invalid name with special characters" << std::endl;
        try {
            context.lookup("app/config/../../../etc/passwd");
        } catch (const std::exception& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
        }
        
        // Test case 4: Lookup non-existent resource
        std::cout << "\\nTest 4: Non-existent resource lookup" << std::endl;
        try {
            context.lookup("nonexistent/resource");
        } catch (const std::exception& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
        }
        
        // Test case 5: Invalid name with URL scheme (should fail)
        std::cout << "\\nTest 5: Invalid name with URL scheme" << std::endl;
        try {
            auto maliciousRes = std::make_shared<StringResource>("malicious");
            context.bind("ldap://attacker.com/exploit", maliciousRes);
        } catch (const std::exception& e) {
            std::cout << "Caught expected exception: " << e.what() << std::endl;
        }
        
        std::cout << "\\nAll tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
