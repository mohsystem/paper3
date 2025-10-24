#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Parses an XML string to find the name of the root element.
 * 
 * This function performs a simple, non-validating parse to find the first
 * opening tag that is not a processing instruction or a comment.
 * It is designed to be safe against XXE attacks as it does not process DTDs
 * or external entities. The caller is responsible for freeing the returned string.
 *
 * @param xmlString The XML content as a C string.
 * @return A dynamically allocated string containing the name of the root element.
 *         Returns NULL if the input is NULL, malformed, empty, or no root 
 *         element is found.
 */
char* findRootElement(const char* xmlString) {
    if (xmlString == NULL) {
        return NULL;
    }

    const char* p = xmlString;
    while (*p != '\0') {
        // Find the first opening bracket
        p = strchr(p, '<');
        if (p == NULL || *(p + 1) == '\0') {
            return NULL; // No tag found or tag is at the very end
        }

        // Check if it's a processing instruction, comment, or closing tag
        char next_char = *(p + 1);
        if (next_char == '?' || next_char == '!' || next_char == '/') {
            p++; // Skip this tag and continue searching
            continue;
        }

        // We found a potential opening tag.
        const char* start_name = p + 1;

        // Skip leading whitespace in tag name, e.g., < root ...>
        while (*start_name != '\0' && isspace((unsigned char)*start_name)) {
            start_name++;
        }
        
        if (*start_name == '\0') {
            return NULL; // Malformed tag
        }

        // Find the end of the element name
        const char* end_name = start_name;
        while (*end_name != '\0' && !isspace((unsigned char)*end_name) && *end_name != '>' && *end_name != '/') {
            end_name++;
        }
        
        // If we hit the end of the string before a delimiter, tag is unclosed/malformed.
        if (*end_name == '\0') {
            return NULL;
        }

        if (end_name == start_name) {
             return NULL; // Empty tag name like <> or < >
        }

        size_t len = end_name - start_name;
        char* rootName = (char*)malloc(len + 1);
        if (rootName == NULL) {
            perror("Failed to allocate memory");
            return NULL; // Memory allocation failed
        }
        
        // Safely copy the string
        memcpy(rootName, start_name, len);
        rootName[len] = '\0';

        return rootName;
    }

    return NULL; // No root element found
}

void runTestCase(const char* testName, const char* xml, const char* expected) {
    printf("--- %s ---\n", testName);
    printf("Input XML: \"%s\"\n", xml);
    char* root = findRootElement(xml);
    printf("Found Root: '%s'\n", root ? root : "NULL");
    printf("Expected:   '%s'\n", expected ? expected : "NULL");

    int pass = 0;
    if (root == NULL && expected == NULL) {
        pass = 1;
    } else if (root != NULL && expected != NULL && strcmp(root, expected) == 0) {
        pass = 1;
    }

    if (pass) {
        printf("Result: PASS\n");
    } else {
        printf("Result: FAIL\n");
    }
    
    free(root); // Free the allocated memory
    printf("\n");
}

int main() {
    // Test Case 1: Simple XML
    runTestCase("Test Case 1: Simple", "<root><child/></root>", "root");

    // Test Case 2: XML with attributes
    runTestCase("Test Case 2: With Attributes", "<project name=\"my-project\"><version>1.0</version></project>", "project");

    // Test Case 3: XML with whitespace and processing instruction
    runTestCase("Test Case 3: Whitespace and PI", "<?xml version=\"1.0\"?> <!-- comment --> < note > <to>Tove</to> </note >", "note");
    
    // Test Case 4: Malformed XML (empty tag)
    runTestCase("Test Case 4: Malformed (empty tag)", "< >", NULL);

    // Test Case 5: Empty/Invalid string
    runTestCase("Test Case 5: Empty String", "", NULL);

    return 0;
}