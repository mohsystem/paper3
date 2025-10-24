#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <cstring> // For strchr

/**
 * Parses the provided string to find the root element's name.
 * This implementation uses a minimal, non-validating manual parsing approach
 * to avoid external library dependencies. It is inherently secure against
 * XXE attacks as it does not interpret DTDs or external entities. It simply
 * searches for the first valid XML element tag.
 *
 * @param xmlString The XML content as a string.
 * @return The name of the root element, or an error message if not found.
 */
std::string getRootElementName(const std::string& xmlString) {
    const char* p = xmlString.c_str();
    while ((p = strchr(p, '<')) != NULL) {
        p++; // Move past '<'

        // Skip whitespace after '<'
        while (*p && isspace(static_cast<unsigned char>(*p))) {
            p++;
        }

        if (*p == '\0') {
            break; // End of string
        }

        // Skip processing instructions, comments, DTDs, and closing tags
        if (*p == '?' || *p == '!' || *p == '/') {
            continue; // Continue search for the next '<'
        }

        // Found the start of the root element tag name
        const char* nameStart = p;
        while (*p && *p != '>' && *p != '/' && !isspace(static_cast<unsigned char>(*p))) {
            p++;
        }
        const char* nameEnd = p;

        if (nameEnd > nameStart) {
            return std::string(nameStart, nameEnd - nameStart);
        }
    }

    return "Error: Root element not found.";
}

int main() {
    std::vector<std::string> testCases = {
        "<root><child/></root>",
        "<notes date=\"2023-10-27\"><note>text</note></notes>",
        "<?xml version=\"1.0\"?><book><title>XML</title></book>",
        " <!-- comment --><data> <item/> </data>",
        "<!DOCTYPE note SYSTEM \"Note.dtd\"><note><to>Tove</to></note>"
    };

    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input XML: " << testCases[i] << std::endl;
        std::string rootElement = getRootElementName(testCases[i]);
        std::cout << "Root Element: " << rootElement << std::endl;
        std::cout << std::endl;
    }
    return 0;
}