#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Parses the JSON string to identify the root element's type.
 *
 * This implementation performs a basic structural check by finding the first and
 * last non-whitespace characters to determine if the document is enclosed
 * in braces `{}` (Object) or brackets `[]` (Array).
 *
 * @param jsonString The string representation of the JSON document.
 * @return A constant string literal: "Object", "Array", or "Invalid".
 */
const char* findRootElement(const char* jsonString) {
    if (jsonString == NULL) {
        return "Invalid";
    }

    const char* first = jsonString;
    // Find the first non-whitespace character
    while (*first && isspace((unsigned char)*first)) {
        first++;
    }

    // If the string is empty or all whitespace
    if (*first == '\0') {
        return "Invalid";
    }

    // Find the last non-whitespace character
    const char* last = first + strlen(first) - 1;
    while (last > first && isspace((unsigned char)*last)) {
        last--;
    }

    if (*first == '{' && *last == '}') {
        return "Object";
    } else if (*first == '[' && *last == ']') {
        return "Array";
    } else {
        return "Invalid";
    }
}

int main() {
    const char* testCases[] = {
        "  { \"name\": \"John\", \"age\": 30 }  ",
        "[ \"apple\", \"banana\", \"cherry\" ]",
        "   ",
        "this is not json",
        "{ \"incomplete\": \"json\"",
        NULL // Sentinel for null test case
    };

    printf("C Test Cases:\n");
    for (int i = 0; testCases[i] != NULL; ++i) {
        printf("Input: \"%s\" -> Output: %s\n", testCases[i], findRootElement(testCases[i]));
    }
    // Test with null input
    printf("Input: null -> Output: %s\n", findRootElement(NULL));
    // Test with empty string
    printf("Input: \"\" -> Output: %s\n", findRootElement(""));

    return 0;
}