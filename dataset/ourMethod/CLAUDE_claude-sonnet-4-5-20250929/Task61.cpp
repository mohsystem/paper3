
#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <stdexcept>

// Lightweight XML parser that only extracts root element name
// Secure implementation: no external entities, bounded parsing, input validation
class SecureXMLRootParser {
private:
    static constexpr size_t MAX_INPUT_SIZE = 1024 * 1024; // 1MB limit
    static constexpr size_t MAX_TAG_LENGTH = 1024;
    
    // Skip whitespace characters safely
    static size_t skipWhitespace(const std::string& xml, size_t pos) {
        while (pos < xml.length() && 
               (xml[pos] == ' ' || xml[pos] == '\\t' || 
                xml[pos] == '\\n' || xml[pos] == '\\r')) {
            pos++;
        }
        return pos;
    }
    
    // Validate tag name characters (alphanumeric, underscore, hyphen, colon for namespaces)
    static bool isValidTagChar(char c) {
        return (c >= 'a' && c <= 'z') || 
               (c >= 'A' && c <= 'Z') || 
               (c >= '0' && c <= '9') || 
               c == '_' || c == '-' || c == ':' || c == '.';
    }
    
public:
    // Extract root element name from XML string
    // Returns empty string if no valid root found
    static std::string getRootElement(const std::string& xml) {
        // Input validation: check size limit
        if (xml.empty() || xml.length() > MAX_INPUT_SIZE) {
            throw std::invalid_argument("Invalid input: XML string empty or exceeds size limit");
        }
        
        size_t pos = 0;
        
        // Skip leading whitespace
        pos = skipWhitespace(xml, pos);
        if (pos >= xml.length()) {
            throw std::invalid_argument("Invalid XML: no content after whitespace");
        }
        
        // Skip XML declaration if present (<?xml ... ?>)
        if (pos + 5 < xml.length() && xml.substr(pos, 5) == "<?xml") {
            pos += 5;
            // Find end of declaration
            size_t endPos = xml.find("?>", pos);
            if (endPos == std::string::npos) {
                throw std::invalid_argument("Invalid XML: unclosed declaration");
            }
            pos = endPos + 2;
            pos = skipWhitespace(xml, pos);
        }
        
        // Skip DOCTYPE declaration if present (reject external entities)
        if (pos + 9 < xml.length() && xml.substr(pos, 9) == "<!DOCTYPE") {
            // Check for external entity references (security measure)
            size_t endPos = xml.find('>', pos);
            if (endPos == std::string::npos) {
                throw std::invalid_argument("Invalid XML: unclosed DOCTYPE");
            }
            std::string doctype = xml.substr(pos, endPos - pos);
            // Reject SYSTEM and PUBLIC external entities
            if (doctype.find("SYSTEM") != std::string::npos || 
                doctype.find("PUBLIC") != std::string::npos) {
                throw std::invalid_argument("Security: external entities not allowed");
            }
            pos = endPos + 1;
            pos = skipWhitespace(xml, pos);
        }
        
        // Skip comments (<!-- ... -->)
        while (pos + 4 < xml.length() && xml.substr(pos, 4) == "<!--") {
            size_t endPos = xml.find("-->", pos + 4);
            if (endPos == std::string::npos) {
                throw std::invalid_argument("Invalid XML: unclosed comment");
            }
            pos = endPos + 3;
            pos = skipWhitespace(xml, pos);
        }
        
        // Now expect root element opening tag
        if (pos >= xml.length() || xml[pos] != '<') {
            throw std::invalid_argument("Invalid XML: expected opening tag");
        }
        pos++;
        
        // Check for processing instruction or other special tags
        if (pos < xml.length() && (xml[pos] == '?' || xml[pos] == '!')) {
            throw std::invalid_argument("Invalid XML: unexpected special tag where root expected");
        }
        
        // Extract tag name with bounds checking
        size_t tagStart = pos;
        size_t tagLength = 0;
        
        // First character must be letter or underscore
        if (pos >= xml.length() || 
            !((xml[pos] >= 'a' && xml[pos] <= 'z') || 
              (xml[pos] >= 'A' && xml[pos] <= 'Z') || 
              xml[pos] == '_')) {
            throw std::invalid_argument("Invalid XML: tag name must start with letter or underscore");
        }
        
        while (pos < xml.length() && tagLength < MAX_TAG_LENGTH) {
            char c = xml[pos];
            
            // Tag name ends at whitespace, '>', '/', or invalid character
            if (c == ' ' || c == '\\t' || c == '\\n' || c == '\\r' || 
                c == '>' || c == '/') {
                break;
            }
            
            // Validate character
            if (!isValidTagChar(c)) {
                throw std::invalid_argument("Invalid XML: illegal character in tag name");
            }
            
            pos++;
            tagLength++;
        }
        
        // Validate tag length
        if (tagLength == 0) {
            throw std::invalid_argument("Invalid XML: empty tag name");
        }
        if (tagLength >= MAX_TAG_LENGTH) {
            throw std::invalid_argument("Invalid XML: tag name too long");
        }
        
        // Extract and return root element name
        std::string rootElement = xml.substr(tagStart, tagLength);
        return rootElement;
    }
};

// Main function with test cases
int main() {
    // Test case 1: Simple XML
    try {
        std::string xml1 = "<root><child>value</child></root>";
        std::string root1 = SecureXMLRootParser::getRootElement(xml1);
        std::cout << "Test 1 - Root element: " << root1 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 1 - Error: " << e.what() << std::endl;
    }
    
    // Test case 2: XML with declaration
    try {
        std::string xml2 = "<?xml version=\\"1.0\\" encoding=\\"UTF-8\\"?><document><section>text</section></document>";
        std::string root2 = SecureXMLRootParser::getRootElement(xml2);
        std::cout << "Test 2 - Root element: " << root2 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 2 - Error: " << e.what() << std::endl;
    }
    
    // Test case 3: XML with whitespace and comments
    try {
        std::string xml3 = "  \\n  <!-- Comment -->\\n  <root-element><child/></root-element>";
        std::string root3 = SecureXMLRootParser::getRootElement(xml3);
        std::cout << "Test 3 - Root element: " << root3 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 3 - Error: " << e.what() << std::endl;
    }
    
    // Test case 4: XML with namespace
    try {
        std::string xml4 = "<ns:root xmlns:ns=\\"http://example.com\\"><ns:child/></ns:root>";
        std::string root4 = SecureXMLRootParser::getRootElement(xml4);
        std::cout << "Test 4 - Root element: " << root4 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 4 - Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Invalid XML (should throw exception)
    try {
        std::string xml5 = "<!DOCTYPE root SYSTEM \\"external.dtd\\"><root/>";
        std::string root5 = SecureXMLRootParser::getRootElement(xml5);
        std::cout << "Test 5 - Root element: " << root5 << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Test 5 - Error (expected): " << e.what() << std::endl;
    }
    
    return 0;
}
