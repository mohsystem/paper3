#include <iostream>
#include <string>
#include <vector>

/**
 * Parses an XML string to find the root element's name using basic string manipulation.
 * This is a simplified parser and may not handle all XML complexities.
 * @param xmlString The string containing the XML document.
 * @return The name of the root element, or an empty string if not found.
 */
std::string getRootElement(const std::string& xmlString) {
    size_t currentPos = 0;
    while (currentPos < xmlString.length()) {
        size_t start = xmlString.find('<', currentPos);
        if (start == std::string::npos) {
            return ""; // No tags found
        }

        // Check for processing instructions (<?), comments (<!--), or DTD (<!DOCTYPE)
        if (start + 1 < xmlString.length()) {
            char nextChar = xmlString[start + 1];
            if (nextChar == '?' || nextChar == '!') {
                size_t end = xmlString.find('>', start);
                if (end != std::string::npos) {
                    currentPos = end + 1;
                    continue; // Skip this tag and find the next one
                } else {
                    return ""; // Malformed XML
                }
            }
        }
        
        // This should be the root element tag
        size_t end = xmlString.find_first_of(" >/", start + 1);
        if (end != std::string::npos) {
            return xmlString.substr(start + 1, end - (start + 1));
        }
        
        break; // Should not happen in well-formed XML
    }
    return ""; // Fallback in case of malformed or empty XML
}


int main() {
    std::vector<std::string> testCases = {
        "<root><child/></root>",
        "<bookstore category=\"COOKING\"><book/></bookstore>",
        "<?xml version=\"1.0\"?><note><to>Tove</to></note>",
        "  <data>  </data>  ",
        "<employees><employee><id>1</id></employee></employees>"
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& xml : testCases) {
        std::cout << "Input: " << xml << std::endl;
        std::cout << "Root Element: " << getRootElement(xml) << std::endl;
        std::cout << "---" << std::endl;
    }

    return 0;
}