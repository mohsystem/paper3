
#include <iostream>
#include <string>
#include <memory>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

json parseAndGetRoot(const std::string& jsonString) {
    if (jsonString.empty()) {
        std::cerr << "Empty JSON string" << std::endl;
        return json();
    }
    
    // Limit input size to prevent resource exhaustion
    if (jsonString.length() > 10000000) {
        std::cerr << "JSON string too large" << std::endl;
        return json();
    }
    
    try {
        json rootElement = json::parse(jsonString);
        return rootElement;
    } catch (const json::parse_error& e) {
        std::cerr << "Invalid JSON format" << std::endl;
        return json();
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON" << std::endl;
        return json();
    }
}

int main() {
    // Test case 1: Simple JSON object
    std::string test1 = R"({"name":"John","age":30})";
    std::cout << "Test 1: " << parseAndGetRoot(test1).dump() << std::endl;
    
    // Test case 2: JSON array
    std::string test2 = "[1,2,3,4,5]";
    std::cout << "Test 2: " << parseAndGetRoot(test2).dump() << std::endl;
    
    // Test case 3: Nested JSON object
    std::string test3 = R"({"user":{"name":"Alice","address":{"city":"NYC"}}})";
    std::cout << "Test 3: " << parseAndGetRoot(test3).dump() << std::endl;
    
    // Test case 4: Empty JSON object
    std::string test4 = "{}";
    std::cout << "Test 4: " << parseAndGetRoot(test4).dump() << std::endl;
    
    // Test case 5: Invalid JSON
    std::string test5 = "{invalid json}";
    std::cout << "Test 5: " << parseAndGetRoot(test5).dump() << std::endl;
    
    return 0;
}
