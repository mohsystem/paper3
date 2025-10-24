#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Helper function to trim whitespace from a string.
// Modifies the string in place and returns a pointer to the start of the trimmed string.
char* trim_whitespace(char* str) {
    if (str == NULL) {
        return NULL;
    }
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == 0) { // All spaces?
        return str;
    }

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    // Write new null terminator character
    end[1] = '\0';

    return str;
}


/**
 * @brief Identifies the root element type of a JSON-like string.
 *
 * This is a non-validating heuristic. It checks the first and last
 * non-whitespace characters and attempts to parse the string as a known
 * literal or a number. It does not perform a full, validating JSON parse.
 *
 * @param jsonString A mutable C string to analyze (it will be modified by trimming).
 * @return A const char* representing the root element type or "invalid".
 */
const char* getRootElementType(char* jsonString) {
    if (jsonString == NULL) {
        return "invalid";
    }
    
    char* trimmed = trim_whitespace(jsonString);

    if (trimmed == NULL || *trimmed == '\0') {
        return "invalid";
    }
    
    size_t len = strlen(trimmed);
    if (len > 1) {
        char firstChar = trimmed[0];
        char lastChar = trimmed[len - 1];
        if (firstChar == '{' && lastChar == '}') return "object";
        if (firstChar == '[' && lastChar == ']') return "array";
        if (firstChar == '"' && lastChar == '"') return "string";
    }

    if (strcmp(trimmed, "true") == 0 || strcmp(trimmed, "false") == 0) {
        return "boolean";
    }
    if (strcmp(trimmed, "null") == 0) {
        return "null";
    }

    // Check for a number using strtod.
    char* endptr;
    strtod(trimmed, &endptr);
    // If strtod consumed the entire string, it's a valid number.
    if (*endptr == '\0') {
        return "number";
    }

    return "invalid";
}

int main() {
    // Note: Test case strings must be mutable for the trim_whitespace function.
    char test1[] = "  { \"key\": \"value\" }  ";
    char test2[] = " [1, 2, 3] ";
    char test3[] = "\"a string\"";
    char test4[] = "123.45";
    char test5[] = "null";
    char test6[] = "123a"; // invalid number
    char test7[] = " { "; // invalid object

    char* test_cases_orig[] = {
        "  { \"key\": \"value\" }  ",
        " [1, 2, 3] ",
        "\"a string\"",
        "123.45",
        "null",
        "123a",
        " { "
    };
    char* test_cases_mut[] = {test1, test2, test3, test4, test5, test6, test7};
    const char* expected[] = {"object", "array", "string", "number", "null", "invalid", "invalid"};
    int num_tests = sizeof(test_cases_mut) / sizeof(test_cases_mut[0]);
    
    printf("Running C tests...\n");
    for (int i = 0; i < num_tests; i++) {
        const char* result = getRootElementType(test_cases_mut[i]);
        const char* status = (strcmp(result, expected[i]) == 0) ? "PASS" : "FAIL";
        printf("Test Case %d: \"%s\" -> %s (Expected: %s) -> %s\n",
               i + 1, test_cases_orig[i], result, expected[i], status);
    }

    return 0;
}