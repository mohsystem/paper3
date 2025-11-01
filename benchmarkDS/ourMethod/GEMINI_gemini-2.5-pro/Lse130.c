#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h> // For ptrdiff_t

// Mock structures to represent web components
typedef struct {
    const char* name;
} Request;

typedef struct {
    const char* content;
} Template;

typedef struct {
    char* body; // Dynamically allocated, must be freed by the caller
} Response;

typedef struct {
    const Request* request;
    const Template* tmpl;
} Environment;


/**
 * @brief Escapes HTML-sensitive characters. Returns a new dynamically allocated string.
 * The caller is responsible for freeing the returned string.
 * This is a critical security function to prevent XSS.
 * Rules#2 & #3: Sanitize output and perform safe buffer operations.
 * @param data The input string to escape.
 * @return A new null-terminated string with escaped characters, or NULL on error.
 */
char* escape_html(const char* data) {
    if (data == NULL) {
        // Return a dynamically allocated empty string for consistency.
        char* empty_str = (char*)malloc(1);
        if (empty_str) empty_str[0] = '\0';
        return empty_str;
    }

    size_t new_len = 0;
    for (size_t i = 0; data[i] != '\0'; ++i) {
        switch (data[i]) {
            case '&':  new_len += 5; break; // &amp;
            case '\"': new_len += 6; break; // &quot;
            case '\'': new_len += 5; break; // &#39;
            case '<':  new_len += 4; break; // &lt;
            case '>':  new_len += 4; break; // &gt;
            default:   new_len += 1;
        }
    }

    char* buffer = (char*)malloc(new_len + 1);
    if (buffer == NULL) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    char* p = buffer;
    for (size_t i = 0; data[i] != '\0'; ++i) {
        switch (data[i]) {
            case '&':  memcpy(p, "&amp;", 5); p += 5; break;
            case '\"': memcpy(p, "&quot;", 6); p += 6; break;
            case '\'': memcpy(p, "&#39;", 5); p += 5; break;
            case '<':  memcpy(p, "&lt;", 4); p += 4; break;
            case '>':  memcpy(p, "&gt;", 4); p += 4; break;
            default:   *p++ = data[i];
        }
    }
    *p = '\0';

    return buffer;
}

/**
 * @brief Replaces the first occurrence of a substring. Returns a new dynamically allocated string.
 * The caller is responsible for freeing the returned string.
 * Rule #3: Adheres to buffer boundaries.
 * @param orig The original string.
 * @param rep The substring to replace.
 * @param with The string to replace with.
 * @return A new dynamically allocated string, or NULL on failure.
 */
char* str_replace(const char* orig, const char* rep, const char* with) {
    if (!orig || !rep || !with) return NULL;

    const char* p = strstr(orig, rep);
    if (!p) { // Substring not found, return a copy of original
        size_t orig_len = strlen(orig);
        char* copy = (char*)malloc(orig_len + 1);
        if(copy) memcpy(copy, orig, orig_len + 1);
        return copy;
    }

    size_t rep_len = strlen(rep);
    size_t with_len = strlen(with);
    size_t orig_len = strlen(orig);
    
    // Check for potential integer overflow before allocation
    if (orig_len - rep_len > (size_t)-1 - with_len) {
        fprintf(stderr, "Error: string replacement size calculation would overflow.\n");
        return NULL;
    }
    size_t final_len = orig_len - rep_len + with_len;
    
    char* result = (char*)malloc(final_len + 1);
    if (result == NULL) return NULL;
    
    ptrdiff_t head_len = p - orig;
    memcpy(result, orig, head_len);
    memcpy(result + head_len, with, with_len);
    
    size_t tail_len = orig_len - head_len - rep_len;
    memcpy(result + head_len + with_len, p + rep_len, tail_len);
    result[final_len] = '\0';
    
    return result;
}

/**
 * @brief Gets name from request, gets template from env, renders, and returns response.
 * @param env The environment containing the request and template.
 * @return A Response object with the rendered template body, which must be freed by the caller.
 */
Response render_response_from_env(const Environment* env) {
    Response res = {NULL};
    if (env == NULL || env->request == NULL || env->tmpl == NULL || env->tmpl->content == NULL) {
        res.body = strdup("Error: Invalid environment."); // strdup is POSIX, but common.
        if (res.body == NULL) perror("strdup failed");
        return res;
    }
    
    const char* name = env->request->name;

    // Rules#1: Ensure all input is validated.
    if (name == NULL || name[0] == '\0') {
        name = "Guest";
    }

    char* escaped_name = escape_html(name);
    if (escaped_name == NULL) {
        res.body = strdup("Error: Failed to escape name.");
        if (res.body == NULL) perror("strdup failed");
        return res;
    }

    res.body = str_replace(env->tmpl->content, "{{name}}", escaped_name);
    
    free(escaped_name); // Cleanup intermediate allocation
    
    if (res.body == NULL) {
        res.body = strdup("Error: Failed to render template.");
        if (res.body == NULL) perror("strdup failed");
    }

    return res;
}

/**
 * @brief Home function that calls the render function.
 * @param env The environment variable.
 * @return The response from the render function. The response body must be freed.
 */
Response home(const Environment* env) {
    return render_response_from_env(env);
}

void run_test(const char* test_name, const char* name_param) {
    printf("%s\n", test_name);

    Request req = {name_param};
    Template tmpl = {"<h1>Hello, {{name}}!</h1>"};
    Environment env = {&req, &tmpl};

    Response res = home(&env);
    
    if (res.body != NULL) {
        printf("  Output: %s\n\n", res.body);
        free(res.body); // IMPORTANT: Free the allocated response body
    } else {
        printf("  Output: (null)\n\n");
    }
}

int main() {
    printf("--- Running C Test Cases ---\n");
    run_test("Test 1: Normal name", "Alice");
    run_test("Test 2: Name with special characters", "Bob & Charlie");
    run_test("Test 3: XSS payload", "<script>alert('xss')</script>");
    run_test("Test 4: Empty name", "");
    run_test("Test 5: Null name parameter", NULL);

    return 0;
}