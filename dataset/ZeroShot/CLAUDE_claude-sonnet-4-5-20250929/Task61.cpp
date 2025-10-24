
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

std::string parseAndGetRootElement(const std::string& xmlString) {
    try {
        // Simple XML parser to extract root element
        size_t start = xmlString.find('<');
        if (start == std::string::npos) {
            return "Error: No opening tag found";
        }
        
        // Skip XML declaration if present
        if (xmlString.substr(start, 5) == "<?xml") {
            start = xmlString.find("?>", start);
            if (start == std::string::npos) {
                return "Error: Invalid XML declaration";
            }
            start = xmlString.find('<', start + 2);
        }
        
        if (start == std::string::npos) {
            return "Error: No root element found";
        }
        
        start++; // Skip the '<'
        
        // Handle namespaces
        size_t end = start;
        while (end < xmlString.length() && 
               xmlString[end] != '>' && 
               xmlString[end] != ' ' && 
               xmlString[end] != '/' &&
               xmlString[end] != '\\t' &&
               xmlString[end] != '\\n') {
            end++;
        }
        
        std::string rootElement = xmlString.substr(start, end - start);
        
        // Remove any whitespace
        rootElement.erase(std::remove_if(rootElement.begin(), rootElement.end(), ::isspace), rootElement.end());
        
        return rootElement.empty() ? "Error: Empty root element" : rootElement;
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main() {
    // Test case 1: Simple XML
    std::string test1 = "<?xml version=\\"1.0\\"?><root><child>value</child></root>";
    std::cout << "Test 1 - Root element: " << parseAndGetRootElement(test1) << std::endl;
    
    // Test case 2: XML with attributes
    std::string test2 = "<?xml version=\\"1.0\\"?><books><book id=\\"1\\">Java</book></books>";
    std::cout << "Test 2 - Root element: " << parseAndGetRootElement(test2) << std::endl;
    
    // Test case 3: Complex XML
    std::string test3 = "<?xml version=\\"1.0\\"?><company><employee><name>John</name></employee></company>";
    std::cout << "Test 3 - Root element: " << parseAndGetRootElement(test3) << std::endl;
    
    // Test case 4: XML with namespaces
    std::string test4 = "<?xml version=\\"1.0\\"?><ns:document xmlns:ns=\\"http://example.com\\"><ns:data>test</ns:data></ns:document>";
    std::cout << "Test 4 - Root element: " << parseAndGetRootElement(test4) << std::endl;
    
    // Test case 5: Single element XML
    std::string test5 = "<?xml version=\\"1.0\\"?><message/>";
    std::cout << "Test 5 - Root element: " << parseAndGetRootElement(test5) << std::endl;
    
    return 0;
}
