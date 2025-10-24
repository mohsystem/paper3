
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <stdexcept>
#include <cstring>

class Task131 {
public:
    // Simple data structure for safe serialization
    struct SafeData {
        std::string name;
        int value;
        
        SafeData() : name(""), value(0) {}
        SafeData(const std::string& n, int v) : name(n), value(v) {}
        
        std::string toString() const {
            return "SafeData{name='" + name + "', value=" + std::to_string(value) + "}";
        }
    };
    
    // Serialize SafeData to string format (simple CSV-like)
    static std::string serializeObject(const SafeData& data) {
        std::ostringstream oss;
        oss << "SAFEDATA|" << data.name.length() << "|" << data.name << "|" << data.value;
        return oss.str();
    }
    
    // Securely deserialize with validation
    static SafeData deserializeSecurely(const std::string& serialized) {
        if (serialized.empty()) {
            throw std::invalid_argument("Invalid serialized data");
        }
        
        std::istringstream iss(serialized);
        std::string type;
        char delimiter;
        size_t nameLength;
        std::string name;
        int value;
        
        // Parse: SAFEDATA|length|name|value
        std::getline(iss, type, '|');
        if (type != "SAFEDATA") {
            throw std::runtime_error("Unauthorized deserialization attempt: " + type);
        }
        
        iss >> nameLength >> delimiter;
        if (delimiter != '|') {
            throw std::runtime_error("Invalid format");
        }
        
        name.resize(nameLength);
        iss.read(&name[0], nameLength);
        
        iss >> delimiter >> value;
        if (delimiter != '|') {
            throw std::runtime_error("Invalid format");
        }
        
        return SafeData(name, value);
    }
    
    // JSON-like serialization (safer alternative)
    static std::string serializeToJSON(const SafeData& data) {
        return "{\\"name\\":\\"" + data.name + "\\",\\"value\\":" + std::to_string(data.value) + "}";
    }
    
    // JSON-like deserialization
    static SafeData deserializeFromJSON(const std::string& json) {
        size_t nameStart = json.find("\\"name\\":\\"") + 8;
        size_t nameEnd = json.find("\\"", nameStart);
        std::string name = json.substr(nameStart, nameEnd - nameStart);
        
        size_t valueStart = json.find("\\"value\\":") + 8;
        size_t valueEnd = json.find("}", valueStart);
        int value = std::stoi(json.substr(valueStart, valueEnd - valueStart));
        
        return SafeData(name, value);
    }
};

int main() {
    std::cout << "=== Secure Deserialization Tests ===" << std::endl << std::endl;
    
    // Test Case 1: Deserialize SafeData object
    try {
        std::cout << "Test 1: Deserialize SafeData object" << std::endl;
        Task131::SafeData original("TestData", 42);
        std::string serialized = Task131::serializeObject(original);
        Task131::SafeData deserialized = Task131::deserializeSecurely(serialized);
        std::cout << "Original: " << original.toString() << std::endl;
        std::cout << "Deserialized: " << deserialized.toString() << std::endl;
        std::cout << "Success!" << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl << std::endl;
    }
    
    // Test Case 2: Deserialize with different values
    try {
        std::cout << "Test 2: Deserialize with different values" << std::endl;
        Task131::SafeData original("SecureData", 12345);
        std::string serialized = Task131::serializeObject(original);
        Task131::SafeData deserialized = Task131::deserializeSecurely(serialized);
        std::cout << "Original: " << original.toString() << std::endl;
        std::cout << "Deserialized: " << deserialized.toString() << std::endl;
        std::cout << "Success!" << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl << std::endl;
    }
    
    // Test Case 3: JSON-based serialization
    try {
        std::cout << "Test 3: JSON-based serialization" << std::endl;
        Task131::SafeData original("JSONData", 999);
        std::string json = Task131::serializeToJSON(original);
        Task131::SafeData deserialized = Task131::deserializeFromJSON(json);
        std::cout << "Original: " << original.toString() << std::endl;
        std::cout << "JSON: " << json << std::endl;
        std::cout << "Deserialized: " << deserialized.toString() << std::endl;
        std::cout << "Success!" << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl << std::endl;
    }
    
    // Test Case 4: Empty string handling
    try {
        std::cout << "Test 4: Empty string handling" << std::endl;
        Task131::SafeData original("", 0);
        std::string serialized = Task131::serializeObject(original);
        Task131::SafeData deserialized = Task131::deserializeSecurely(serialized);
        std::cout << "Original: " << original.toString() << std::endl;
        std::cout << "Deserialized: " << deserialized.toString() << std::endl;
        std::cout << "Success!" << std::endl << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl << std::endl;
    }
    
    // Test Case 5: Invalid format detection
    try {
        std::cout << "Test 5: Invalid format detection (should fail)" << std::endl;
        std::string invalid = "INVALID|DATA|FORMAT";
        Task131::SafeData deserialized = Task131::deserializeSecurely(invalid);
        std::cout << "This should not print" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Expected error caught: " << e.what() << std::endl;
        std::cout << "Success!" << std::endl << std::endl;
    }
    
    return 0;
}
