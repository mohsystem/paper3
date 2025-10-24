
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

std::string getRootElement(const std::string& xmlString) {
    try {
        size_t start = xmlString.find('<');
        if (start == std::string::npos) {
            return "Error: No XML tags found";
        }
        
        // Skip XML declaration if present
        if (xmlString.substr(start, 5) == "<?xml") {
            start = xmlString.find("?>", start);
            if (start != std::string::npos) {
                start = xmlString.find('<', start + 2);
            }
        }
        
        if (start == std::string::npos || start >= xmlString.length()) {
            return "Error: No root element found";
        }
        
        start++; // Skip the '<' character
        size_t end = start;
        
        // Find the end of the tag name (space, '>', or '/')
        while (end < xmlString.length() && 
               xmlString[end] != '>' && 
               xmlString[end] != ' ' && 
               xmlString[end] != '/') {
            end++;
        }
        
        return xmlString.substr(start, end - start);
    } catch (...) {
        return "Error: Parsing failed";
    }
}

int main() {
    // Test case 1: Simple XML
    std::string xml1 = "<?xml version=\\"1.0\\"?><root><child>value</child></root>";
    std::cout << "Test 1 - Root element: " << getRootElement(xml1) << std::endl;
    
    // Test case 2: XML with attributes
    std::string xml2 = "<?xml version=\\"1.0\\"?><book id=\\"1\\"><title>Java Programming</title></book>";
    std::cout << "Test 2 - Root element: " << getRootElement(xml2) << std::endl;
    
    // Test case 3: XML with nested elements
    std::string xml3 = "<?xml version=\\"1.0\\"?><catalog><product><name>Item1</name></product></catalog>";
    std::cout << "Test 3 - Root element: " << getRootElement(xml3) << std::endl;
    
    // Test case 4: XML with different root
    std::string xml4 = "<?xml version=\\"1.0\\"?><students><student><name>John</name></student></students>";
    std::cout << "Test 4 - Root element: " << getRootElement(xml4) << std::endl;
    
    // Test case 5: Simple root without declaration
    std::string xml5 = "<data><item>test</item></data>";
    std::cout << "Test 5 - Root element: " << getRootElement(xml5) << std::endl;
    
    return 0;
}
