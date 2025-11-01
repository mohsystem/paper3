#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Environment structure
typedef struct {
    const char* template_str;
} Environment;

/**
 * Renders a template from the environment with a given name.
 * Note: The returned string is dynamically allocated and must be freed by the caller.
 * @param env The environment containing the template.
 * @param name The name to insert into the template.
 * @return A dynamically allocated string with the rendered template, or NULL on failure.
 */
char* render_response_from_env(const Environment* env, const char* name) {
    // The function gets the template from the environment.
    const char* tpl = env->template_str;
    const char* placeholder = "{name}";
    const char* placeholder_pos = strstr(tpl, placeholder);

    // If placeholder is not found, just return a copy of the template
    if (placeholder_pos == NULL) {
        char* response = (char*)malloc(strlen(tpl) + 1);
        if (response == NULL) return NULL;
        strcpy(response, tpl);
        return response;
    }

    // The function renders the template with the name parameter from the request.
    size_t name_len = strlen(name);
    size_t placeholder_len = strlen(placeholder);
    size_t prefix_len = placeholder_pos - tpl;
    const char* suffix_start = placeholder_pos + placeholder_len;
    size_t suffix_len = strlen(suffix_start);

    size_t new_len = prefix_len + name_len + suffix_len;
    char* response = (char*)malloc(new_len + 1);
    if (response == NULL) return NULL; // Allocation failed

    // Build the new string
    memcpy(response, tpl, prefix_len);
    memcpy(response + prefix_len, name, name_len);
    memcpy(response + prefix_len + name_len, suffix_start, suffix_len + 1); // +1 to copy null terminator
    
    // The function returns a response with the rendered template.
    return response;
}

/**
 * Main entry point for the "home" route.
 * @param env The environment.
 * @param name The name from the request.
 * @return The response from the rendering function. Caller must free memory.
 */
char* home(const Environment* env, const char* name) {
    // The home function calls the render_response_from_env function with the env variable.
    // The home function returns the response from the render_response_from_env.
    return render_response_from_env(env, name);
}

int main() {
    // The render_response_from_env function takes an environment as an argument.
    Environment env = {"Hello, {name}!"};

    const char* test_cases[] = {"Alice", "Bob", "Charlie", "World", ""};
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    printf("C Test Cases:\n");
    for (int i = 0; i < num_test_cases; i++) {
        // The function gets the name parameter from the user request. (Simulated)
        const char* name = test_cases[i];
        char* response = home(&env, name);

        if (response) {
            printf("Test Case %d (Name: '%s'): %s\n", i + 1, name, response);
            free(response); // Free the memory allocated by home/render function
        } else {
            printf("Test Case %d (Name: '%s'): Memory allocation failed!\n", i + 1, name);
        }
    }

    return 0;
}