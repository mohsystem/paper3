#include <iostream>
#include <string>
#include <cctype>

/**
 * Parses an XML string and returns the name of the root element.
 * NOTE: This is a simplified, non-validating parser for demonstration purposes.
 * It does not use a dedicated XML library to remain self-contained. It finds the first
 * opening tag that isn't a comment, processing instruction, or closing tag.
 *
 * @param xmlString The std::string containing the XML document.
 * @return The name of the root element, or an empty string if not found or on error.
 */
std::string getRootElement(const std::string& xmlString) {
    size_t startPos = 0;
    while ((startPos = xmlString.find('<', startPos)) != std::string::npos) {
        if (startPos + 1 < xmlString.length()) {
            char nextChar = xmlString[startPos + 1];
            // Skip comments, processing instructions, and closing tags
            if (nextChar != '!' && nextChar != '?' && nextChar != '/') {
                // This is likely the root element tag
                size_t endPos = xmlString.find_first_of(" >/", startPos + 1);
                if (endPos != std::string::npos) {
                    return xmlString.substr(startPos + 1, endPos - (startPos + 1));
                }
                break; // Malformed tag, stop searching
            }
        }
        startPos++; // Move past the current '<' to continue searching
    }
    return ""; // Return empty string if no root element is found
}

int main() {
    std::string testCases[] = {
        "<root><child>data</child></root>",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><bookstore><book category=\"cooking\"><title lang=\"en\">Everyday Italian</title></book></bookstore>",
        "<!-- This is a comment --><project name=\"xml-parser\"><version>1.0</version></project>",
        "<note><to>Tove</to><from>Jani</from><heading>Reminder</heading></note>",
        "This is not a valid XML string."
    };

    std::cout << "Running CPP Test Cases:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Test Case " << (i + 1) << ": \"" << testCases[i] << "\"" << std::endl;
        std::string root = getRootElement(testCases[i]);
        if (root.empty()) {
            std::cout << "Root Element: Error parsing XML or not found" << std::endl;
        } else {
            std::cout << "Root Element: " << root << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}