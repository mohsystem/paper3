#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simulates an environment that holds a template
typedef struct {
    const char* template_str;
} Environment;

// Simulates a user request, simplified to just hold the name
typedef struct {
    const char* name;
} UserRequest;

// Simulates a response
typedef struct {
    char* body; // Dynamically allocated string
} Response;

/**
 * @brief A safe string replacement function.
 * Replaces all occurrences of a substring. Returns a new dynamically allocated string.
 * The caller is responsible for freeing the returned memory.
 * Returns NULL on failure.
 */
char* replace_all(const char* str, const char* from, const char* to) {
    if (str == NULL || from == NULL || to == NULL) return NULL;

    size_t from_len = strlen(from);
    if (from_len == 0) {
        char* result = strdup(str);
        return result;
    }
    size_t to_len = strlen(to);

    // Count occurrences of 'from' to calculate final string size
    size_t count = 0;
    const char* p = str;
    while ((p = strstr(p, from))) {
        count++;
        p += from_len;
    }

    size_t new_len = strlen(str) + count * (to_len - from_len);
    char* result = (char*)malloc(new_len + 1);
    if (result == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    char* q = result;
    p = str;
    while (1) {
        const char* next_from = strstr(p, from);
        if (next_from == NULL) {
            strcpy(q, p);
            break;
        }
        // Copy part before 'from'
        memcpy(q, p, next_from - p);
        q += (next_from - p);
        // Copy 'to'
        memcpy(q, to, to_len);
        q += to_len;
        // Move p past the found 'from'
        p = next_from + from_len;
    }

    return result;
}


/**
 * @brief Renders a response from a template.
 * @param env Pointer to the environment.
 * @param request Pointer to the user request.
 * @return A Response object. The 'body' must be freed by the caller.
 */
Response render_response_from_env(const Environment* env, const UserRequest* request) {
    Response res = { NULL };
    
    // 1. Get the name parameter from the user request.
    const char* name = (request->name != NULL) ? request->name : "Guest";
    // In a real web app, user input must be sanitized/escaped.

    // 2. Get the template from the environment.
    const char* tpl = env->template_str;

    // 3. Render the template with the name parameter.
    res.body = replace_all(tpl, "{{name}}", name);

    // 4. Return a response with the rendered template.
    return res;
}

/**
 * @brief Simulates a home route handler.
 * @param env Pointer to the environment.
 * @param request Pointer to the user request.
 * @return The response from the rendering function.
 */
Response home(const Environment* env, const UserRequest* request) {
    return render_response_from_env(env, request);
}

int main() {
    // Setup a shared environment for all requests
    Environment env = {"Hello, {{name}}! Welcome to our site."};

    // --- Test Cases ---

    // Test Case 1: Standard name
    UserRequest request1 = {"Alice"};
    Response response1 = home(&env, &request1);
    printf("Test Case 1: %s\n", response1.body ? response1.body : "Error");
    free(response1.body);

    // Test Case 2: Another name
    UserRequest request2 = {"Bob"};
    Response response2 = home(&env, &request2);
    printf("Test Case 2: %s\n", response2.body ? response2.body : "Error");
    free(response2.body);

    // Test Case 3: Name with numbers
    UserRequest request3 = {"Charlie7"};
    Response response3 = home(&env, &request3);
    printf("Test Case 3: %s\n", response3.body ? response3.body : "Error");
    free(response3.body);

    // Test Case 4: No "name" parameter provided (defaults to "Guest")
    UserRequest request4 = {NULL};
    Response response4 = home(&env, &request4);
    printf("Test Case 4: %s\n", response4.body ? response4.body : "Error");
    free(response4.body);

    // Test Case 5: Name that could be an XSS attempt in a real web app
    UserRequest request5 = {"<script>alert('xss')</script>"};
    Response response5 = home(&env, &request5);
    printf("Test Case 5: %s\n", response5.body ? response5.body : "Error");
    free(response5.body);

    return 0;
}