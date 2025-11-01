#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* template_str;
} Environment;

/**
 * @brief Escapes special HTML characters to prevent Cross-Site Scripting (XSS).
 * 
 * @param input The string to escape.
 * @return A new dynamically allocated string with escaped content. The caller must free this string.
 */
char* html_escape(const char* input) {
    if (!input) return NULL;
    
    size_t input_len = strlen(input);
    size_t escaped_len = input_len;

    // First pass: calculate the final length of the escaped string
    for (size_t i = 0; i < input_len; ++i) {
        switch (input[i]) {
            case '&':  escaped_len += 4; break; // &amp;
            case '<':  escaped_len += 3; break; // &lt;
            case '>':  escaped_len += 3; break; // &gt;
            case '"':  escaped_len += 5; break; // &quot;
            case '\'': escaped_len += 4; break; // &#39;
        }
    }

    char* escaped = (char*)malloc(escaped_len + 1);
    if (!escaped) return NULL;

    // Second pass: build the escaped string
    char* p = escaped;
    for (size_t i = 0; i < input_len; ++i) {
        switch (input[i]) {
            case '&':  p += sprintf(p, "&amp;");  break;
            case '<':  p += sprintf(p, "&lt;");   break;
            case '>':  p += sprintf(p, "&gt;");   break;
            case '"':  p += sprintf(p, "&quot;"); break;
            case '\'': p += sprintf(p, "&#39;");  break;
            default:   *p++ = input[i];
        }
    }
    *p = '\0';
    return escaped;
}


/**
 * @brief Renders a template from an environment with a given name parameter.
 * 
 * @param env Pointer to the environment struct.
 * @param name The name parameter from the request.
 * @return A new dynamically allocated string with the rendered response. The caller must free it.
 */
char* render_response_from_env(const Environment* env, const char* name) {
    if (!env || !env->template_str || !name) return NULL;
    
    // Secure: Escape the name parameter
    char* escaped_name = html_escape(name);
    if (!escaped_name) return NULL;

    const char* placeholder = "{{ name }}";
    char* placeholder_pos = strstr(env->template_str, placeholder);

    // If placeholder is not found, just return a copy of the template.
    if (!placeholder_pos) {
        char* result = (char*)malloc(strlen(env->template_str) + 1);
        if(result) strcpy(result, env->template_str);
        free(escaped_name);
        return result;
    }
    
    size_t placeholder_len = strlen(placeholder);
    size_t escaped_name_len = strlen(escaped_name);
    size_t pre_placeholder_len = placeholder_pos - env->template_str;
    size_t post_placeholder_len = strlen(placeholder_pos + placeholder_len);
    
    // Allocate memory for the final rendered string
    size_t result_len = pre_placeholder_len + escaped_name_len + post_placeholder_len;
    char* result = (char*)malloc(result_len + 1);
    if (!result) {
        free(escaped_name);
        return NULL;
    }

    // Build the result string
    // 1. Copy the part before the placeholder
    strncpy(result, env->template_str, pre_placeholder_len);
    result[pre_placeholder_len] = '\0';

    // 2. Append the escaped name
    strcat(result, escaped_name);

    // 3. Append the part after the placeholder
    strcat(result, placeholder_pos + placeholder_len);
    
    free(escaped_name);
    return result;
}

/**
 * @brief Simulates a home route handler that renders a response.
 * 
 * @param env Pointer to the environment struct.
 * @param name The name parameter.
 * @return The rendered response from the renderer. Caller must free this memory.
 */
char* home(const Environment* env, const char* name) {
    return render_response_from_env(env, name);
}


int main() {
    // 1. Setup the environment with a template.
    Environment env = {"<h1>Hello, {{ name }}!</h1>"};

    // 2. Define test cases.
    const char* test_cases[] = {
        "World",
        "Jane Doe",
        "",
        "<script>alert('XSS')</script>",
        "<b>Bold</b>"
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    // 3. Run test cases.
    for (int i = 0; i < num_test_cases; ++i) {
        char* response = home(&env, test_cases[i]);
        printf("Test Case %d (name=%s):\n", i + 1, test_cases[i]);
        if (response) {
            printf("Output: %s\n\n", response);
            free(response); // IMPORTANT: Free the allocated memory
        } else {
            printf("Output: (Error during rendering)\n\n");
        }
    }

    return 0;
}