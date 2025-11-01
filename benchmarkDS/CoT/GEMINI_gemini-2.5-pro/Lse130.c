#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simulating the environment and request structures
typedef struct {
    char* key;
    char* value;
} KeyValuePair;

typedef struct {
    KeyValuePair* params;
    int param_count;
} Request;

typedef struct {
    Request request;
    char* template_str;
} Environment;

/**
 * Escapes characters in a string to be safe for use in HTML, preventing XSS.
 * NOTE: The caller is responsible for freeing the returned string.
 * @param input The string to escape.
 * @return A new, dynamically allocated, escaped string. Returns NULL on allocation failure.
 */
char* htmlEscape(const char* input) {
    if (!input) return NULL;

    size_t new_len = strlen(input);
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&': new_len += 4; break; // &amp;
            case '<': new_len += 3; break; // &lt;
            case '>': new_len += 3; break; // &gt;
            case '"': new_len += 5; break; // &quot;
            case '\'': new_len += 4; break; // &#39;
        }
    }

    char* escaped = (char*)malloc(new_len + 1);
    if (!escaped) return NULL;

    size_t j = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&': strcpy(escaped + j, "&amp;"); j += 5; break;
            case '<': strcpy(escaped + j, "&lt;"); j += 4; break;
            case '>': strcpy(escaped + j, "&gt;"); j += 4; break;
            case '"': strcpy(escaped + j, "&quot;"); j += 6; break;
            case '\'': strcpy(escaped + j, "&#39;"); j += 5; break;
            default: escaped[j++] = input[i];
        }
    }
    escaped[j] = '\0';
    return escaped;
}

/**
 * Replaces a placeholder in a template string.
 * NOTE: The caller is responsible for freeing the returned string.
 * @param tmpl The template string.
 * @param placeholder The placeholder to replace (e.g., "{name}").
 * @param value The value to substitute.
 * @return A new, dynamically allocated, rendered string. Returns NULL on failure.
 */
char* renderTemplate(const char* tmpl, const char* placeholder, const char* value) {
    char* pos = strstr(tmpl, placeholder);
    if (!pos) {
        char* copy = (char*)malloc(strlen(tmpl) + 1);
        if(copy) strcpy(copy, tmpl);
        return copy;
    }

    size_t placeholder_len = strlen(placeholder);
    size_t value_len = strlen(value);
    size_t result_len = strlen(tmpl) - placeholder_len + value_len;
    
    char* result = (char*)malloc(result_len + 1);
    if (!result) return NULL;

    strncpy(result, tmpl, pos - tmpl);
    result[pos - tmpl] = '\0';
    strcat(result, value);
    strcat(result, pos + placeholder_len);
    
    return result;
}


/**
 * Renders a response from an environment object. It gets a name from the request,
 * securely renders it into a template, and returns an HTTP-like response.
 * NOTE: The caller is responsible for freeing the returned string.
 * @param env An Environment object.
 * @return A string representing the rendered response. Returns NULL on failure.
 */
char* render_response_from_env(const Environment* env) {
    const char* name = "Guest"; // Default value
    
    // Get the name parameter from the request
    for (int i = 0; i < env->request.param_count; i++) {
        if (strcmp(env->request.params[i].key, "name") == 0) {
            name = env->request.params[i].value;
            break;
        }
    }

    const char* template_str = env->template_str;
    
    char* safe_name = htmlEscape(name);
    if (!safe_name) return NULL;

    char* rendered_template = renderTemplate(template_str, "{name}", safe_name);
    free(safe_name);
    if (!rendered_template) return NULL;

    const char* headers = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
    size_t final_len = strlen(headers) + strlen(rendered_template);
    char* response = (char*)malloc(final_len + 1);
    if (!response) {
        free(rendered_template);
        return NULL;
    }
    
    strcpy(response, headers);
    strcat(response, rendered_template);
    
    free(rendered_template);
    return response;
}

/**
 * Home function that calls render_response_from_env.
 * NOTE: The caller is responsible for freeing the returned string.
 * @param env An Environment object.
 * @return The response from render_response_from_env.
 */
char* home(const Environment* env) {
    return render_response_from_env(env);
}

void run_test(const char* test_name, Environment* env) {
    printf("--- %s ---\n", test_name);
    char* response = home(env);
    if (response) {
        printf("%s\n\n", response);
        free(response);
    } else {
        printf("Failed to generate response.\n\n");
    }
}

int main() {
    char* template_str = "<h1>Hello, {name}!</h1>";

    // Test Case 1: Simple name
    KeyValuePair params1[] = {{"name", "Alice"}};
    Environment env1 = {{params1, 1}, template_str};
    run_test("Test Case 1", &env1);

    // Test Case 2: Name with a space
    KeyValuePair params2[] = {{"name", "Bob Smith"}};
    Environment env2 = {{params2, 1}, template_str};
    run_test("Test Case 2", &env2);

    // Test Case 3: Name with characters that need escaping
    KeyValuePair params3[] = {{"name", "O'Malley & Sons"}};
    Environment env3 = {{params3, 1}, template_str};
    run_test("Test Case 3", &env3);

    // Test Case 4: XSS attack attempt
    KeyValuePair params4[] = {{"name", "<script>alert('pwned')</script>"}};
    Environment env4 = {{params4, 1}, template_str};
    run_test("Test Case 4 (XSS Prevention)", &env4);

    // Test Case 5: Missing name parameter
    Environment env5 = {{NULL, 0}, template_str};
    run_test("Test Case 5 (Default Name)", &env5);

    return 0;
}