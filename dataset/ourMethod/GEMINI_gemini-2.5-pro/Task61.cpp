#include <iostream>
#include <string>
#include <vector>
#include <cctype>

/**
 * @brief Parses an XML string to find the name of the root element.
 * 
 * This function performs a simple, non-validating parse to find the first
 * opening tag that is not a processing instruction or a comment.
 * It is designed to be safe against XXE attacks as it does not process DTDs
 * or external entities.
 *
 * @param xmlString The XML content as a string.
 * @return The name of the root element. Returns an empty string if the input
 *         is malformed, empty, or no root element is found.
 */
std::string findRootElement(const std::string& xmlString) {
    size_t pos = 0;
    while (pos < xmlString.length()) {
        // Find the first opening bracket
        pos = xmlString.find('<', pos);
        if (pos == std::string::npos || pos + 1 >= xmlString.length()) {
            return ""; // No tag found or tag is at the very end
        }

        // Check if it's a processing instruction, comment, or closing tag
        char next_char = xmlString[pos + 1];
        if (next_char == '?' || next_char == '!' || next_char == '/') {
            pos++; // Skip this tag and continue searching
            continue;
        }

        // We found a potential opening tag.
        size_t start_name = pos + 1;

        // Skip leading whitespace in tag name, e.g., < root ...>
        while (start_name < xmlString.length() && isspace(static_cast<unsigned char>(xmlString[start_name]))) {
            start_name++;
        }

        if (start_name >= xmlString.length()) {
            return ""; // Malformed tag
        }

        // Find the end of the element name (ends with space, '>', or '/')
        size_t end_name = xmlString.find_first_of(" \t\n\r/>", start_name);
        if (end_name == std::string::npos) {
            return ""; // Malformed XML, tag not closed properly in the string
        }
        
        if (end_name == start_name) {
            return ""; // Empty tag name, e.g., <> or < >
        }

        // Extract and return the name
        return xmlString.substr(start_name, end_name - start_name);
    }
    return ""; // No root element found
}

void runTestCase(const std::string& testName, const std::string& xml, const std::string& expected) {
    std::cout << "--- " << testName << " ---" << std::endl;
    std::cout << "Input XML: \"" << xml << "\"" << std::endl;
    std::string root = findRootElement(xml);
    std::cout << "Found Root: '" << root << "'" << std::endl;
    std::cout << "Expected:   '" << expected << "'" << std::endl;
    if (root == expected) {
        std::cout << "Result: PASS" << std::endl;
    } else {
        std::cout << "Result: FAIL" << std::endl;
    }
    std::cout << std::endl;
}

int main() {
    // Test Case 1: Simple XML
    runTestCase("Test Case 1: Simple", "<root><child/></root>", "root");

    // Test Case 2: XML with attributes
    runTestCase("Test Case 2: With Attributes", "<project name=\"my-project\"><version>1.0</version></project>", "project");

    // Test Case 3: XML with whitespace and processing instruction
    runTestCase("Test Case 3: Whitespace and PI", "<?xml version=\"1.0\"?> <!-- comment --> < note > <to>Tove</to> </note >", "note");
    
    // Test Case 4: Malformed XML (empty tag)
    runTestCase("Test Case 4: Malformed (empty tag)", "< >", "");

    // Test Case 5: Empty/Invalid string
    runTestCase("Test Case 5: Empty String", "", "");

    return 0;
}