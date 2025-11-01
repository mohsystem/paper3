#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple structure to simulate a request
typedef struct {
    char* name;
} Request;

// A simple structure to simulate the environment
typedef struct {
    Request request;
    const char* template_str;
} Environment;

/**
 * @brief Escapes HTML special characters in a string to prevent XSS.
 * @param str The string to escape.
 * @return A new, dynamically allocated string with escaped characters.
 *         The caller is responsible for freeing this memory.
 */
char* escapeHtml(const char* str) {
    if (!str) return NULL;

    size_t new_len = strlen(str);
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '&') new_len += 4; // &amp;
        else if (str[i] == '<' || str[i] == '>') new_len += 3; // &lt; or &gt;
        else if (str[i] == '"') new_len += 5; // &quot;
        else if (str[i] == '\'') new_len += 4; // &#39;
    }

    char* escaped_str = (char*)malloc(new_len + 1);
    if (!escaped_str) return NULL;

    size_t j = 0;
    for (size_t i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '&':  strcpy(escaped_str + j, "&amp;");  j += 5; break;
            case '<':  strcpy(escaped_str + j, "&lt;");   j += 4; break;
            case '>':  strcpy(escaped_str + j, "&gt;");   j += 4; break;
            case '"':  strcpy(escaped_str + j, "&quot;"); j += 6; break;
            case '\'': strcpy(escaped_str + j, "&#39;");  j += 5; break;
            default:   escaped_str[j++] = str[i]; break;
        }
    }
    escaped_str[j] = '\0';
    return escaped_str;
}

/**
 * @brief Renders a template from an environment with a name parameter.
 * @param env The environment containing the request and template.
 * @return A new, dynamically allocated response string. The caller
 *         is responsible for freeing this memory.
 */
char* render_response_from_env(Environment env) {
    const char* name = "Guest";
    // Get the name parameter from the request
    if (env.request.name && strlen(env.request.name) > 0) {
        name = env.request.name;
    }

    // Get the template from the environment
    const char* template_str = env.template_str;
    const char* placeholder = "{{ name }}";

    // SECURITY: Escape the user-provided name to prevent XSS attacks
    char* safe_name = escapeHtml(name);
    if (!safe_name) return NULL; // Allocation failed

    // Render the template by replacing the placeholder
    char* placeholder_pos = strstr(template_str, placeholder);
    if (!placeholder_pos) {
        // If placeholder not found, just return a copy of the template
        char* response = strdup(template_str);
        free(safe_name);
        return response;
    }

    size_t before_len = placeholder_pos - template_str;
    size_t safe_name_len = strlen(safe_name);
    size_t after_len = strlen(placeholder_pos + strlen(placeholder));

    size_t total_len = before_len + safe_name_len + after_len;
    char* response = (char*)malloc(total_len + 1);
    if (!response) {
        free(safe_name);
        return NULL; // Allocation failed
    }

    // Build the final string
    memcpy(response, template_str, before_len);
    memcpy(response + before_len, safe_name, safe_name_len);
    memcpy(response + before_len + safe_name_len, placeholder_pos + strlen(placeholder), after_len);
    response[total_len] = '\0';

    free(safe_name); // Clean up the escaped string
    return response;
}

/**
 * @brief Simulates a "home" endpoint controller.
 * @param env The environment for the request.
 * @return The response generated from the environment.
 */
char* home(Environment env) {
    return render_response_from_env(env);
}

// In C, a "main class" concept doesn't exist. We use a main function.
// The name Lse129 is not used as a class/struct name for the main logic.
int main() {
    const char* template_str = "<h1>Hello, {{ name }}!</h1>";
    char* response;

    // Test Case 1: Normal name
    Environment env1 = {{"Alice"}, template_str};
    response = home(env1);
    printf("Test Case 1 (Normal): %s\n", response);
    free(response);

    // Test Case 2: XSS attack attempt
    Environment env2 = {{"<script>alert('XSS')</script>"}, template_str};
    response = home(env2);
    printf("Test Case 2 (XSS): %s\n", response);
    free(response);

    // Test Case 3: Empty name parameter
    Environment env3 = {{""}, template_str};
    response = home(env3);
    printf("Test Case 3 (Empty): %s\n", response);
    free(response);

    // Test Case 4: NULL name parameter
    Environment env4 = {{NULL}, template_str};
    response = home(env4);
    printf("Test Case 4 (NULL): %s\n", response);
    free(response);

    // Test Case 5: Name with special characters
    Environment env5 = {{"Bob & Charlie"}, template_str};
    response = home(env5);
    printf("Test Case 5 (Special Chars): %s\n", response);
    free(response);

    return 0;
}