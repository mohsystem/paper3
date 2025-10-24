#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Escapes special characters in a string for use in HTML.
 * This is crucial to prevent Cross-Site Scripting (XSS) attacks.
 * This function dynamically allocates memory for the result, which
 * must be freed by the caller.
 *
 * @param input The raw string to be escaped.
 * @return A pointer to the newly allocated, HTML-escaped string, or NULL on failure.
 */
char* htmlEscape(const char* input) {
    if (!input) {
        char* empty_str = (char*)malloc(1);
        if (empty_str) empty_str[0] = '\0';
        return empty_str;
    }

    size_t new_len = 0;
    for (size_t i = 0; input[i] != '\0'; ++i) {
        switch (input[i]) {
            case '&': new_len += 5; break; // &amp;
            case '"': new_len += 6; break; // &quot;
            case '\'': new_len += 5; break; // &#39;
            case '<': new_len += 4; break; // &lt;
            case '>': new_len += 4; break; // &gt;
            default:  new_len += 1; break;
        }
    }

    char* escaped_str = (char*)malloc(new_len + 1);
    if (!escaped_str) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    char* writer = escaped_str;
    for (size_t i = 0; input[i] != '\0'; ++i) {
        switch (input[i]) {
            case '&':  memcpy(writer, "&amp;", 5); writer += 5; break;
            case '"':  memcpy(writer, "&quot;", 6); writer += 6; break;
            case '\'': memcpy(writer, "&#39;", 5); writer += 5; break;
            case '<':  memcpy(writer, "&lt;", 4); writer += 4; break;
            case '>':  memcpy(writer, "&gt;", 4); writer += 4; break;
            default:   *writer++ = input[i]; break;
        }
    }
    *writer = '\0';

    return escaped_str;
}

/**
 * Generates an HTML page displaying the user-provided (and now escaped) input.
 * Note: C has no standard library for a web server. This function
 * generates the HTML content that a web server would serve.
 * This function dynamically allocates memory for the result, which
 * must be freed by the caller.
 *
 * @param userInput The user-provided string.
 * @return A pointer to the newly allocated string containing the full HTML page, or NULL on failure.
 */
char* generateWebPage(const char* userInput) {
    char* escapedInput = htmlEscape(userInput);
    if (!escapedInput) {
        return NULL;
    }

    const char* template1 =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>User Input Display</title>\n"
        "    <meta charset=\"UTF-8\">\n"
        "</head>\n"
        "<body>\n"
        "    <h1>User-Supplied Input:</h1>\n"
        "    <p style=\"border: 1px solid black; padding: 10px; background-color: #f0f0f0;\">\n        ";
    const char* template2 =
        "\n    </p>\n"
        "</body>\n"
        "</html>";

    size_t len1 = strlen(template1);
    size_t len2 = strlen(template2);
    size_t len_esc = strlen(escapedInput);
    size_t total_len = len1 + len_esc + len2;

    char* page_content = (char*)malloc(total_len + 1);
    if (!page_content) {
        perror("Failed to allocate memory for page content");
        free(escapedInput);
        return NULL;
    }

    int written = snprintf(page_content, total_len + 1, "%s%s%s", template1, escapedInput, template2);
    free(escapedInput);

    if (written < 0 || (size_t)written >= total_len + 1) {
        fprintf(stderr, "snprintf error when generating page.\n");
        free(page_content);
        return NULL;
    }

    return page_content;
}

void runTestCase(const char* testName, const char* input) {
    printf("--- %s ---\n", testName);
    char* page = generateWebPage(input);
    if (page) {
        printf("%s\n\n", page);
        free(page);
    } else {
        printf("Failed to generate page for test case: %s\n\n", testName);
    }
}

int main() {
    // C has no standard library for networking. The core security task is
    // to correctly escape user input before embedding it in HTML. The test
    // cases below demonstrate this by generating the HTML as a string and
    // printing it to the console, ensuring memory is properly managed.

    // Test Case 1: Simple text
    runTestCase("Test Case 1 (Simple Text)", "Hello, world!");

    // Test Case 2: Text with HTML characters
    runTestCase("Test Case 2 (HTML Chars)", "<p>This is a paragraph.</p>");

    // Test Case 3: XSS attack attempt
    runTestCase("Test Case 3 (XSS Attempt)", "<script>alert('XSS');</script>");

    // Test Case 4: Text with quotes and ampersand
    runTestCase("Test Case 4 (Quotes & Ampersand)", "AT&T's new motto is \"Rethink Possible\".");

    // Test Case 5: Empty input
    runTestCase("Test Case 5 (Empty Input)", "");

    return 0;
}