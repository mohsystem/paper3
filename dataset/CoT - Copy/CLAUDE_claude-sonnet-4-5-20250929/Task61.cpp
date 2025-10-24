
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

// Note: For production use, consider using libxml2 with proper security settings
// This implementation provides basic XML parsing with security considerations

std::string parseAndGetRootElement(const std::string& xmlString) {
    if (xmlString.empty() || xmlString.find_first_not_of(" \\t\\n\\r") == std::string::npos) {
        return "Error: Empty or null XML string";
    }
    
    try {
        // Find the start of the root element (after XML declaration if present)
        size_t rootStart = xmlString.find('<');
        if (rootStart == std::string::npos) {
            return "Error: No XML element found";
        }
        
        // Skip XML declaration if present
        if (xmlString.substr(rootStart, 5) == "<?xml") {
            rootStart = xmlString.find("?>", rootStart);
            if (rootStart == std::string::npos) {
                return "Error: Malformed XML declaration";
            }
            rootStart = xmlString.find('<', rootStart + 2);
        }
        
        if (rootStart == std::string::npos) {
            return "Error: No root element found";
        }
        
        // Extract root element name
        size_t nameStart = rootStart + 1;
        size_t nameEnd = nameStart;
        
        while (nameEnd < xmlString.length() && 
               xmlString[nameEnd] != '>' && 
               xmlString[nameEnd] != ' ' && 
               xmlString[nameEnd] != '\\t' &&
               xmlString[nameEnd] != '\\n' &&
               xmlString[nameEnd] != '\\r') {
            nameEnd++;
        }
        
        if (nameEnd > nameStart) {
            return xmlString.substr(nameStart, nameEnd - nameStart);
        }
        
        return "Error: Could not extract root element name";
        
    } catch (const std::exception& e) {
        return std::string("Error: ") + e.what();
    }
}

int main() {
    // Test case 1: Simple XML
    std::string test1 = "<?xml version=\\"1.0\\"?><root><child>data</child></root>";
    std::cout << "Test 1 - Root element: " << parseAndGetRootElement(test1) << std::endl;
    
    // Test case 2: Complex XML
    std::string test2 = "<?xml version=\\"1.0\\" encoding=\\"UTF-8\\"?><books><book id=\\"1\\"><title>Java</title></book></books>";
    std::cout << "Test 2 - Root element: " << parseAndGetRootElement(test2) << std::endl;
    
    // Test case 3: Different root element
    std::string test3 = "<catalog><item>Product</item></catalog>";
    std::cout << "Test 3 - Root element: " << parseAndGetRootElement(test3) << std::endl;
    
    // Test case 4: Malformed XML
    std::string test4 = "<root><unclosed>";
    std::cout << "Test 4 - Root element: " << parseAndGetRootElement(test4) << std::endl;
    
    // Test case 5: Empty string
    std::string test5 = "";
    std::cout << "Test 5 - Root element: " << parseAndGetRootElement(test5) << std::endl;
    
    return 0;
}
