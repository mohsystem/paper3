#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * Parses the provided string to identify the type of the root JSON element.
 *
 * @param jsonString The string representing a JSON document.
 * @return A constant string indicating the root element's type ("Object", "Array", "String",
 *         "Number", "Boolean", "Null", or "Invalid").
 */
const char* getRootElementType(const char* jsonString) {
    if (jsonString == NULL) {
        return "Invalid";
    }

    // Find the start of non-whitespace characters
    const char* start = jsonString;
    while (*start && isspace((unsigned char)*start)) {
        start++;
    }

    if (*start == '\0') { // String is empty or all whitespace
        return "Invalid";
    }

    // Find the end of non-whitespace characters
    const char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    size_t len = end - start + 1;

    // Check based on first and last characters
    if (*start == '{' && *end == '}') {
        return "Object";
    }
    if (*start == '[' && *end == ']') {
        return "Array";
    }
    if (*start == '"' && *end == '"') {
        return "String";
    }

    // Check for literals by comparing the trimmed substring
    if (len == 4 && strncmp(start, "true", 4) == 0) {
        return "Boolean";
    }
    if (len == 5 && strncmp(start, "false", 5) == 0) {
        return "Boolean";
    }
    if (len == 4 && strncmp(start, "null", 4) == 0) {
        return "Null";
    }

    // Check for number by trying to convert the trimmed substring
    // Create a temporary null-terminated string for strtod
    char temp[len + 1];
    strncpy(temp, start, len);
    temp[len] = '\0';

    char* endptr;
    strtod(temp, &endptr);
    // If strtod consumed the entire temporary string, it's a number
    if (*endptr == '\0') {
        return "Number";
    }

    return "Invalid";
}

int main() {
    const char* testCases[] = {
        "  { \"name\": \"John\", \"age\": 30 }  ", // Test case 1: Object
        "[1, \"apple\", true, null]",              // Test case 2: Array
        "\"This is a root string.\"",              // Test case 3: String
        "  -123.45e6  ",                          // Test case 4: Number
        "null"                                     // Test case 5: Null
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < numTestCases; ++i) {
        const char* json = testCases[i];
        const char* result = getRootElementType(json);
        printf("Test Case %d: \"%s\" -> %s\n", (i + 1), json, result);
    }

    return 0;
}