#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
char* renderResponseFromEnv(const char* templateStr, const char* name_param);
char* home(const char* templateStr, const char* name_param);
char* htmlEscape(const char* data);

/**
 * @brief Escapes HTML special characters in a string.
 * This is a security measure to prevent Cross-Site Scripting (XSS).
 * The caller is responsible for freeing the returned string.
 *
 * @param data The input string to escape. Can be NULL.
 * @return char* A newly allocated, HTML-escaped string, or NULL on error.
 */
char* htmlEscape(const char* data) {
    if (!data) {
        char* empty_str = (char*)malloc(1);
        if (empty_str) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    size_t data_len = strlen(data);
    size_t escaped_len = data_len;
    
    // First pass: calculate required length
    for (size_t i = 0; i < data_len; ++i) {
        switch (data[i]) {
            case '&':  escaped_len += 4; break; // &amp;
            case '\"': escaped_len += 5; break; // &quot;
            case '\'': escaped_len += 5; break; // &#x27;
            case '<':  escaped_len += 3; break; // &lt;
            case '>':  escaped_len += 3; break; // &gt;
        }
    }
    
    char* escaped_str = (char*)malloc(escaped_len + 1);
    if (!escaped_str) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }
    
    // Second pass: build the escaped string
    char* p_out = escaped_str;
    for (size_t i = 0; i < data_len; ++i) {
        switch (data[i]) {
            case '&':  memcpy(p_out, "&amp;", 5); p_out += 5; break;
            case '\"': memcpy(p_out, "&quot;", 6); p_out += 6; break;
            case '\'': memcpy(p_out, "&#x27;", 6); p_out += 6; break;
            case '<':  memcpy(p_out, "&lt;", 4); p_out += 4; break;
            case '>':  memcpy(p_out, "&gt;", 4); p_out += 4; break;
            default:   *p_out++ = data[i]; break;
        }
    }
    *p_out = '\0';
    
    return escaped_str;
}

/**
 * @brief Renders a response from a template and a name parameter.
 * The caller is responsible for freeing the returned string.
 *
 * @param templateStr The template string, e.g., "<h1>Hello, {{name}}!</h1>".
 * @param name_param The name to insert into the template. Can be NULL.
 * @return char* A newly allocated string with the rendered response, or NULL on error.
 */
char* renderResponseFromEnv(const char* templateStr, const char* name_param) {
    if (!templateStr) {
        return NULL;
    }

    const char* name = (name_param && *name_param) ? name_param : "Guest";

    char* sanitized_name = htmlEscape(name);
    if (!sanitized_name) {
        return NULL; // htmlEscape failed
    }

    const char* placeholder = "{{name}}";
    char* placeholder_pos = strstr(templateStr, placeholder);

    if (!placeholder_pos) {
        // If placeholder not found, just return a copy of the template
        size_t template_len = strlen(templateStr);
        char* result = (char*)malloc(template_len + 1);
        if (result) {
            memcpy(result, templateStr, template_len + 1);
        }
        free(sanitized_name);
        return result;
    }

    size_t placeholder_len = strlen(placeholder);
    size_t sanitized_name_len = strlen(sanitized_name);
    size_t template_len = strlen(templateStr);
    
    size_t part1_len = placeholder_pos - templateStr;
    size_t part2_len = template_len - (part1_len + placeholder_len);

    // Check for integer overflow before calculating total length
    size_t total_len = part1_len + sanitized_name_len + part2_len;
    if (total_len < part1_len || total_len < sanitized_name_len || total_len < part2_len) {
        fprintf(stderr, "Error: size_t overflow detected.\n");
        free(sanitized_name);
        return NULL;
    }
    
    char* result = (char*)malloc(total_len + 1);
    if (!result) {
        perror("Failed to allocate memory for response");
        free(sanitized_name);
        return NULL;
    }

    // Safely construct the final string
    memcpy(result, templateStr, part1_len);
    memcpy(result + part1_len, sanitized_name, sanitized_name_len);
    memcpy(result + part1_len + sanitized_name_len, placeholder_pos + placeholder_len, part2_len);
    result[total_len] = '\0';

    free(sanitized_name);
    return result;
}

/**
 * @brief A home function that simulates a controller action.
 *
 * @param templateStr The template string.
 * @param name_param The name parameter from the request.
 * @return char* The response from the renderer. Caller must free.
 */
char* home(const char* templateStr, const char* name_param) {
    return renderResponseFromEnv(templateStr, name_param);
}

void run_test(const char* test_name, const char* templateStr, const char* name_param) {
    char* response = home(templateStr, name_param);
    if (response) {
        printf("Test (%s): %s\n", test_name, response);
        free(response);
    } else {
        printf("Test (%s): FAILED (NULL response)\n", test_name);
    }
}

int main() {
    const char* env_template = "<h1>Hello, {{name}}!</h1>";

    printf("--- 5 Test Cases ---\n");

    // Test Case 1: Normal name
    run_test("Normal", env_template, "Alice");

    // Test Case 2: Name with HTML characters (potential XSS)
    run_test("XSS", env_template, "<script>alert('xss')</script>");

    // Test Case 3: Empty name parameter
    run_test("Empty", env_template, "");

    // Test Case 4: Missing name parameter (NULL)
    run_test("Missing", env_template, NULL);

    // Test Case 5: Name with other special characters
    run_test("Special Chars", env_template, "Bob & Charlie's \"Shop\"");

    return 0;
}