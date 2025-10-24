#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Parses an XML string and returns the name of the root element.
 * NOTE: This is a simplified, non-validating parser for demonstration purposes.
 * It finds the first opening tag that isn't a comment, processing instruction, or closing tag.
 *
 * @param xmlString The character array containing the XML document.
 * @return A dynamically allocated string with the name of the root element.
 *         The caller is responsible for freeing this memory. Returns NULL on failure.
 */
char* getRootElement(const char* xmlString) {
    const char* start = xmlString;
    while ((start = strchr(start, '<')) != NULL) {
        if (*(start + 1) != '\0') {
            char nextChar = *(start + 1);
            // Skip comments, processing instructions, and closing tags
            if (nextChar != '!' && nextChar != '?' && nextChar != '/') {
                const char* tagNameStart = start + 1;
                const char* tagNameEnd = tagNameStart;
                while (*tagNameEnd != '\0' && *tagNameEnd != ' ' && *tagNameEnd != '>' && *tagNameEnd != '/') {
                    tagNameEnd++;
                }

                size_t len = tagNameEnd - tagNameStart;
                if (len > 0) {
                    char* rootName = (char*)malloc(len + 1);
                    if (rootName == NULL) {
                        perror("Failed to allocate memory");
                        return NULL;
                    }
                    strncpy(rootName, tagNameStart, len);
                    rootName[len] = '\0';
                    return rootName;
                }
            }
        }
        start++; // Move to the next character to continue search
    }
    return NULL; // Return NULL if no root element is found
}

int main() {
    const char* testCases[] = {
        "<root><child>data</child></root>",
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?><bookstore><book category=\"cooking\"><title lang=\"en\">Everyday Italian</title></book></bookstore>",
        "<!-- This is a comment --><project name=\"xml-parser\"><version>1.0</version></project>",
        "<note><to>Tove</to><from>Jani</from><heading>Reminder</heading></note>",
        "This is not a valid XML string."
    };

    printf("Running C Test Cases:\n");
    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d: \"%s\"\n", i + 1, testCases[i]);
        char* root = getRootElement(testCases[i]);
        if (root != NULL) {
            printf("Root Element: %s\n", root);
            free(root); // Free the dynamically allocated memory
        } else {
            printf("Root Element: Error parsing XML or not found\n");
        }
        printf("\n");
    }

    return 0;
}