
#include <iostream>
#include <string>
#include <regex>
#include <stdexcept>
#include <sstream>

class UserData {
private:
    std::string name;
    int age;
    std::string email;

public:
    UserData(const std::string& n, int a, const std::string& e) 
        : name(n), age(a), email(e) {}
    
    std::string getName() const { return name; }
    int getAge() const { return age; }
    std::string getEmail() const { return email; }
    
    std::string toString() const {
        std::stringstream ss;
        ss << "UserData{name='" << name << "', age=" << age << ", email='" << email << "'}";
        return ss.str();
    }
};

// Simple JSON parser for demonstration (in production, use a library like nlohmann/json)
class SimpleJsonParser {
public:
    static std::string extractString(const std::string& json, const std::string& key) {
        std::string searchKey = "\\"" + key + "\\":\\"";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) return "";
        
        pos += searchKey.length();
        size_t endPos = json.find("\\"", pos);
        if (endPos == std::string::npos) return "";
        
        return json.substr(pos, endPos - pos);
    }
    
    static int extractInt(const std::string& json, const std::string& key) {
        std::string searchKey = "\\"" + key + "\\":";
        size_t pos = json.find(searchKey);
        if (pos == std::string::npos) throw std::runtime_error("Key not found");
        
        pos += searchKey.length();
        size_t endPos = json.find_first_of(",}", pos);
        std::string numStr = json.substr(pos, endPos - pos);
        
        return std::stoi(numStr);
    }
};

UserData deserializeUserData(const std::string& jsonData) {
    if (jsonData.empty()) {
        throw std::invalid_argument("Input data cannot be empty");
    }
    
    try {
        // Extract fields from JSON
        std::string name = SimpleJsonParser::extractString(jsonData, "name");
        int age = SimpleJsonParser::extractInt(jsonData, "age");
        std::string email = SimpleJsonParser::extractString(jsonData, "email");
        
        // Validate name
        if (name.empty()) {
            throw std::invalid_argument("Name cannot be empty");
        }
        
        // Validate age
        if (age < 0 || age > 150) {
            throw std::invalid_argument("Age must be between 0 and 150");
        }
        
        // Validate email
        std::regex emailPattern("^[A-Za-z0-9+_.-]+@(.+)$");
        if (!std::regex_match(email, emailPattern)) {
            throw std::invalid_argument("Invalid email format");
        }
        
        return UserData(name, age, email);
        
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("Deserialization error: ") + e.what());
    }
}

int main() {
    std::cout << "=== Secure Deserialization Test Cases ===\\n\\n";
    
    // Test Case 1: Valid data
    try {
        std::string json1 = "{\\"name\\":\\"John Doe\\",\\"age\\":30,\\"email\\":\\"john@example.com\\"}";
        UserData user1 = deserializeUserData(json1);
        std::cout << "Test 1 - Valid data: " << user1.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test Case 2: Valid data with different values
    try {
        std::string json2 = "{\\"name\\":\\"Jane Smith\\",\\"age\\":25,\\"email\\":\\"jane@test.com\\"}";
        UserData user2 = deserializeUserData(json2);
        std::cout << "Test 2 - Valid data: " << user2.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test Case 3: Invalid age (negative)
    try {
        std::string json3 = "{\\"name\\":\\"Bob\\",\\"age\\":-5,\\"email\\":\\"bob@example.com\\"}";
        UserData user3 = deserializeUserData(json3);
        std::cout << "Test 3 - Should fail: " << user3.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 - Expected failure: " << e.what() << std::endl;
    }
    
    // Test Case 4: Invalid email format
    try {
        std::string json4 = "{\\"name\\":\\"Alice\\",\\"age\\":28,\\"email\\":\\"invalid-email\\"}";
        UserData user4 = deserializeUserData(json4);
        std::cout << "Test 4 - Should fail: " << user4.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 - Expected failure: " << e.what() << std::endl;
    }
    
    // Test Case 5: Invalid age (too high)
    try {
        std::string json5 = "{\\"name\\":\\"Charlie\\",\\"age\\":200,\\"email\\":\\"charlie@test.com\\"}";
        UserData user5 = deserializeUserData(json5);
        std::cout << "Test 5 - Should fail: " << user5.toString() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Expected failure: " << e.what() << std::endl;
    }
    
    return 0;
}
