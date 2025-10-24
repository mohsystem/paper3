#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Parses an XML string to find the root element's name using basic string manipulation.
 * The caller is responsible for freeing the returned string.
 * This is a simplified parser and may not handle all XML complexities.
 * @param xmlString The string containing the XML document.
 * @return A dynamically allocated string with the root element name, or NULL on failure.
 */
char* getRootElement(const char* xmlString) {
    const char* current_pos = xmlString;
    while (*current_pos != '\0') {
        const char* start_tag = strchr(current_pos, '<');
        if (start_tag == NULL) {
            return NULL; // No tags found
        }
        
        // Check for processing instructions (<?), comments (<!--), or DTD (<!DOCTYPE)
        if (*(start_tag + 1) == '?' || *(start_tag + 1) == '!') {
            const char* end_tag = strchr(start_tag, '>');
            if (end_tag != NULL) {
                current_pos = end_tag + 1;
                continue; // Skip this tag and find the next one
            } else {
                return NULL; // Malformed XML
            }
        }

        // This should be the root element tag. Find its end.
        const char* p = start_tag + 1;
        while (*p != '\0' && !isspace((unsigned char)*p) && *p != '>' && *p != '/') {
            p++;
        }
        
        size_t len = p - (start_tag + 1);
        if (len > 0) {
            char* root_name = (char*)malloc(len + 1);
            if (root_name == NULL) {
                return NULL; // Memory allocation failed
            }
            strncpy(root_name, start_tag + 1, len);
            root_name[len] = '\0';
            return root_name;
        }

        break; // Should not happen in well-formed XML
    }

    return NULL; // Fallback in case of malformed or empty XML
}


int main() {
    const char* testCases[] = {
        "<root><child/></root>",
        "<bookstore category=\"COOKING\"><book/></bookstore>",
        "<?xml version=\"1.0\"?><note><to>Tove</to></note>",
        "  <data>  </data>  ",
        "<employees><employee><id>1</id></employee></employees>"
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    printf("C Test Cases:\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Input: %s\n", testCases[i]);
        char* root = getRootElement(testCases[i]);
        if (root != NULL) {
            printf("Root Element: %s\n", root);
            free(root); // Free the allocated memory
        } else {
            printf("Root Element: Not found or error\n");
        }
        printf("---\n");
    }

    return 0;
}