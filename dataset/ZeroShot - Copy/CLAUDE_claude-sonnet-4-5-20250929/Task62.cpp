
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

json parseAndGetRoot(const string& jsonString) {
    if (jsonString.empty()) {
        return nullptr;
    }
    
    try {
        json rootElement = json::parse(jsonString);
        return rootElement;
    } catch (const json::parse_error& e) {
        cerr << "Invalid JSON syntax: " << e.what() << endl;
        return nullptr;
    }
}

int main() {
    // Test case 1: Simple JSON object
    string test1 = R"({"name": "John", "age": 30})";
    cout << "Test 1 Root: " << parseAndGetRoot(test1) << endl;
    
    // Test case 2: JSON array
    string test2 = "[1, 2, 3, 4, 5]";
    cout << "Test 2 Root: " << parseAndGetRoot(test2) << endl;
    
    // Test case 3: Nested JSON object
    string test3 = R"({"user": {"name": "Alice", "address": {"city": "NYC"}}})";
    cout << "Test 3 Root: " << parseAndGetRoot(test3) << endl;
    
    // Test case 4: Empty JSON object
    string test4 = "{}";
    cout << "Test 4 Root: " << parseAndGetRoot(test4) << endl;
    
    // Test case 5: Invalid JSON (should handle gracefully)
    string test5 = "{invalid json}";
    cout << "Test 5 Root: " << parseAndGetRoot(test5) << endl;
    
    return 0;
}
