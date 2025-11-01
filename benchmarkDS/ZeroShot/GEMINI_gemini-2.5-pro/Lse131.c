#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple representation of an environment that holds a template
typedef struct {
    const char* template_str;
} Environment;

/**
 * Securely escapes HTML special characters to prevent XSS.
 * @param input The raw string from user input.
 * @return A new, dynamically allocated escaped string. The caller must free this memory.
 */
char* escape_html(const char* input) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    size_t new_len = strlen(input);
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&': new_len += 4; break; // &amp;
            case '<':
            case '>': new_len += 3; break; // &lt; &gt;
            case '"': new_len += 5; break; // &quot;
            case '\'': new_len += 4; break; // &#39;
        }
    }

    char* escaped = (char*)malloc(new_len + 1);
    if (!escaped) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    size_t j = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&':  memcpy(&escaped[j], "&amp;", 5); j += 5; break;
            case '<':  memcpy(&escaped[j], "&lt;", 4); j += 4; break;
            case '>':  memcpy(&escaped[j], "&gt;", 4); j += 4; break;
            case '"':  memcpy(&escaped[j], "&quot;", 6); j += 6; break;
            case '\'': memcpy(&escaped[j], "&#39;", 5); j += 5; break;
            default:   escaped[j++] = input[i]; break;
        }
    }
    escaped[j] = '\0';
    return escaped;
}


/**
 * Renders a response from a template environment.
 * Note: This function allocates memory for the result, which must be freed by the caller.
 *
 * @param env The template environment.
 * @param name The name parameter from the user request.
 * @return A new dynamically allocated string with the rendered template.
 */
char* render_response_from_env(const Environment* env, const char* name) {
    // Securely escape the user-provided name parameter
    char* safe_name = escape_html(name);
    if (!safe_name) return NULL;

    const char* tpl = env->template_str;
    const char* placeholder = "{{name}}";
    
    char* placeholder_pos = strstr(tpl, placeholder);

    if (placeholder_pos == NULL) {
        // If placeholder is not found, just return a copy of the template
        char* result = (char*)malloc(strlen(tpl) + 1);
        if (result) strcpy(result, tpl);
        free(safe_name);
        return result;
    }

    size_t tpl_part1_len = placeholder_pos - tpl;
    size_t placeholder_len = strlen(placeholder);
    size_t safe_name_len = strlen(safe_name);
    size_t tpl_part2_len = strlen(tpl) - tpl_part1_len - placeholder_len;

    size_t total_len = tpl_part1_len + safe_name_len + tpl_part2_len;
    char* result = (char*)malloc(total_len + 1);
    
    if (!result) {
        perror("Failed to allocate memory for response");
        free(safe_name);
        return NULL;
    }

    // Build the final string
    strncpy(result, tpl, tpl_part1_len);
    strcpy(result + tpl_part1_len, safe_name);
    strcpy(result + tpl_part1_len + safe_name_len, placeholder_pos + placeholder_len);
    
    free(safe_name); // Clean up the escaped name string
    return result;
}

/**
 * Simulates a "home" endpoint/controller function.
 * @return A dynamically allocated string with the response, must be freed by the caller.
 */
char* home(const Environment* env, const char* name) {
    return render_response_from_env(env, name);
}

int main() {
    // The environment is created once, holding our template.
    Environment env = {"<h1>Hello, {{name}}!</h1>"};

    // --- Test Cases ---
    const char* test_names[] = {
        "World",                               // Test Case 1: Normal input
        "<script>alert('XSS');</script>",      // Test Case 2: Malicious script input
        "",                                    // Test Case 3: Empty input
        "John \"Johnny\" Doe & Sons' Store",   // Test Case 4: Input with special characters
        "A very long name with multiple spaces and characters to test rendering" // Test Case 5: Long input
    };
    int num_tests = sizeof(test_names) / sizeof(test_names[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Test Case %d (Input: \"%s\"):\n", i + 1, test_names[i]);
        char* response = home(&env, test_names[i]);
        if (response) {
            printf("Output: %s\n\n", response);
            free(response); // IMPORTANT: Free the memory allocated by home()
        } else {
            printf("Output: (Error: Failed to generate response)\n\n");
        }
    }

    return 0;
}