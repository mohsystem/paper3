#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structs to represent the environment and request
typedef struct {
    char* name;
} Request;

typedef struct {
    Request request;
    char* template_str;
} Environment;

/**
 * Renders a template from an Environment object.
 * The caller is responsible for freeing the returned string.
 * @param env Pointer to the Environment object.
 * @return A dynamically allocated string with the rendered template.
 */
char* render_response_from_env(const Environment* env) {
    // The function gets the name parameter from the request.
    char* name = env->request.name;

    // The function gets the template from the environment.
    char* tpl = env->template_str;
    char* placeholder = "{{name}}";

    // Find the placeholder in the template
    char* pos = strstr(tpl, placeholder);

    if (pos == NULL) {
        // If placeholder not found, just return a copy of the template
        char* response = (char*)malloc(strlen(tpl) + 1);
        if(response == NULL) return NULL;
        strcpy(response, tpl);
        return response;
    }

    // Calculate lengths
    size_t tpl_len = strlen(tpl);
    size_t name_len = strlen(name);
    size_t placeholder_len = strlen(placeholder);
    
    // Calculate new string length and allocate memory
    size_t response_len = tpl_len - placeholder_len + name_len;
    char* response = (char*)malloc(response_len + 1);
    if (response == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    // The function renders the template with the name parameter.
    // 1. Copy the part before the placeholder
    size_t prefix_len = pos - tpl;
    strncpy(response, tpl, prefix_len);
    response[prefix_len] = '\0';

    // 2. Concatenate the name
    strcat(response, name);

    // 3. Concatenate the part after the placeholder
    strcat(response, pos + placeholder_len);

    // The function returns a response with the rendered template.
    return response;
}

/**
 * Calls the render_response_from_env function.
 * The caller is responsible for freeing the returned string.
 * @param env Pointer to the Environment object.
 * @return The response from the render function.
 */
char* home(const Environment* env) {
    return render_response_from_env(env);
}

int main() {
    // 5 test cases
    Environment testCases[5];

    // Test Case 1
    testCases[0].request.name = "Alice";
    testCases[0].template_str = "Hello, {{name}}! Welcome to our site.";

    // Test Case 2
    testCases[1].request.name = "Bob";
    testCases[1].template_str = "Greetings, {{name}}!";

    // Test Case 3
    testCases[2].request.name = "Charlie";
    testCases[2].template_str = "Dear {{name}}, your profile is updated.";

    // Test Case 4: Name with spaces
    testCases[3].request.name = "David Smith";
    testCases[3].template_str = "User: {{name}}";

    // Test Case 5: Empty name
    testCases[4].request.name = "";
    testCases[4].template_str = "Hello, {{name}}! You are a guest.";

    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        char* response = home(&testCases[i]);
        printf("  Input Env: { request: { name: \"%s\" }, template: \"%s\" }\n", 
               testCases[i].request.name, testCases[i].template_str);
        if (response) {
            printf("  Response: %s\n\n", response);
            free(response); // Free the dynamically allocated memory
        } else {
            printf("  Response: (null)\n\n");
        }
    }

    return 0;
}