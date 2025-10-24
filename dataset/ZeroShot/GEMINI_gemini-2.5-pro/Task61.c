#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Parses the provided string to find the root element's name.
 * This implementation uses a minimal, non-validating manual parsing approach
 * to avoid external library dependencies. It is inherently secure against
 * XXE attacks as it does not interpret DTDs or external entities. It simply
 * searches for the first valid XML element tag.
 *
 * @param xmlString The XML content as a string.
 * @return A dynamically allocated string containing the root element name.
 *         The caller is responsible for freeing this memory. Returns an error
 *         message string on failure. Returns NULL if memory allocation fails.
 */
char* getRootElementName(const char* xmlString) {
    const char* p = xmlString;
    while ((p = strchr(p, '<')) != NULL) {
        p++; // Move past '<'

        // Skip whitespace after '<'
        while (*p && isspace((unsigned char)*p)) {
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
        while (*p && *p != '>' && *p != '/' && !isspace((unsigned char)*p)) {
            p++;
        }
        const char* nameEnd = p;

        if (nameEnd > nameStart) {
            size_t len = nameEnd - nameStart;
            char* rootName = (char*)malloc(len + 1);
            if (rootName == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                return NULL;
            }
            strncpy(rootName, nameStart, len);
            rootName[len] = '\0';
            return rootName;
        }
    }
    
    // If no root element is found, return an error string.
    const char* errorStr = "Error: Root element not found.";
    char* errorMsg = (char*)malloc(strlen(errorStr) + 1);
    if (errorMsg == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }
    strcpy(errorMsg, errorStr);
    return errorMsg;
}

int main() {
    const char* testCases[] = {
        "<root><child/></root>",
        "<notes date=\"2023-10-27\"><note>text</note></notes>",
        "<?xml version=\"1.0\"?><book><title>XML</title></book>",
        " <!-- comment --><data> <item/> </data>",
        "<!DOCTYPE note SYSTEM \"Note.dtd\"><note><to>Tove</to></note>"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; i++) {
        printf("Test Case %d:\n", i + 1);
        printf("Input XML: %s\n", testCases[i]);
        char* rootElement = getRootElementName(testCases[i]);
        if (rootElement != NULL) {
            printf("Root Element: %s\n", rootElement);
            free(rootElement); // Free the memory allocated by the function
        } else {
            printf("Root Element: (Memory allocation failed)\n");
        }
        printf("\n");
    }
    return 0;
}