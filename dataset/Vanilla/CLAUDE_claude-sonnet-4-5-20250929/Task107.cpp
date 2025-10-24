
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <typeinfo>

// Note: C++ does not have native JNDI support as it's a Java technology.
// This implementation simulates JNDI functionality using a map-based registry.

class JNDISimulator {
private:
    std::map<std::string, std::shared_ptr<void>> registry;
    std::map<std::string, std::string> typeRegistry;

public:
    template<typename T>
    bool bindResource(const std::string& jndiName, const T& resource) {
        try {
            registry[jndiName] = std::make_shared<T>(resource);
            typeRegistry[jndiName] = typeid(T).name();
            std::cout << "Successfully bound: " << jndiName << std::endl;
            return true;
        } catch (const std::exception& e) {
            std::cerr << "Failed to bind: " << jndiName << std::endl;
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }

    template<typename T>
    std::shared_ptr<T> performJNDILookup(const std::string& jndiName) {
        try {
            auto it = registry.find(jndiName);
            if (it != registry.end()) {
                return std::static_pointer_cast<T>(it->second);
            } else {
                std::cerr << "JNDI Lookup failed for: " << jndiName << std::endl;
                std::cerr << "Error: Resource not found" << std::endl;
                return nullptr;
            }
        } catch (const std::exception& e) {
            std::cerr << "JNDI Lookup failed for: " << jndiName << std::endl;
            std::cerr << "Error: " << e.what() << std::endl;
            return nullptr;
        }
    }

    void listBindings(const std::string& contextName) {
        std::cout << "Bindings in context: " << contextName << std::endl;
        for (const auto& pair : registry) {
            if (pair.first.find(contextName) == 0) {
                std::cout << "  Name: " << pair.first 
                         << ", Type: " << typeRegistry[pair.first] << std::endl;
            }
        }
    }

    bool unbindResource(const std::string& jndiName) {
        try {
            auto it = registry.find(jndiName);
            if (it != registry.end()) {
                registry.erase(it);
                typeRegistry.erase(jndiName);
                std::cout << "Successfully unbound: " << jndiName << std::endl;
                return true;
            } else {
                std::cerr << "Resource not found: " << jndiName << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Failed to unbind: " << jndiName << std::endl;
            std::cerr << "Error: " << e.what() << std::endl;
            return false;
        }
    }
};

int main() {
    std::cout << "=== JNDI Lookup Simulation (C++) ===\\n" << std::endl;
    
    JNDISimulator jndi;
    
    // Test Case 1: Bind and lookup a String
    std::cout << "Test Case 1: String Resource" << std::endl;
    std::string testString = "Hello JNDI World";
    jndi.bindResource("test/string", testString);
    auto result1 = jndi.performJNDILookup<std::string>("test/string");
    if (result1) {
        std::cout << "Retrieved: " << *result1 << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 2: Bind and lookup an Integer
    std::cout << "Test Case 2: Integer Resource" << std::endl;
    int testInteger = 12345;
    jndi.bindResource("test/integer", testInteger);
    auto result2 = jndi.performJNDILookup<int>("test/integer");
    if (result2) {
        std::cout << "Retrieved: " << *result2 << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 3: Bind and lookup a double
    std::cout << "Test Case 3: Double Resource" << std::endl;
    double testDouble = 3.14159;
    jndi.bindResource("test/double", testDouble);
    auto result3 = jndi.performJNDILookup<double>("test/double");
    if (result3) {
        std::cout << "Retrieved: " << *result3 << std::endl;
    }
    std::cout << std::endl;
    
    // Test Case 4: List all bindings
    std::cout << "Test Case 4: List Bindings" << std::endl;
    jndi.listBindings("test");
    std::cout << std::endl;
    
    // Test Case 5: Lookup non-existent resource
    std::cout << "Test Case 5: Non-existent Resource" << std::endl;
    auto result5 = jndi.performJNDILookup<std::string>("test/nonexistent");
    std::cout << "Retrieved: " << (result5 ? "Found" : "NULL") << std::endl;
    
    return 0;
}
