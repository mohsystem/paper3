#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The function escape() converts special characters to HTML-safe sequences.
// NOTE: The caller is responsible for freeing the returned memory.
char* escape(const char* str) {
    if (str == NULL) return NULL;
    
    size_t new_len = strlen(str);
    for (size_t i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '<': case '>': new_len += 3; break; // &lt; &gt;
            case '&': new_len += 4; break; // &amp;
            case '"': case '\'': new_len += 5; break; // &quot; &#39;
        }
    }

    char* result = (char*)malloc(new_len + 1);
    if (!result) return NULL;

    char* p = result;
    for (size_t i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
            case '<': p += sprintf(p, "&lt;"); break;
            case '>': p += sprintf(p, "&gt;"); break;
            case '&': p += sprintf(p, "&amp;"); break;
            case '"': p += sprintf(p, "&quot;"); break;
            case '\'': p += sprintf(p, "&#39;"); break;
            default: *p++ = str[i];
        }
    }
    *p = '\0';
    return result;
}

// The function hello() returns a string with the text Hello followed by the username.
// NOTE: The caller is responsible for freeing the returned memory.
char* hello(const char* username) {
    char* escaped_username = escape(username);
    if (!escaped_username) return NULL;

    const char* prefix = "Hello, ";
    const char* suffix = "!";
    size_t result_len = strlen(prefix) + strlen(escaped_username) + strlen(suffix) + 1;
    
    char* result = (char*)malloc(result_len);
    if (result) {
        sprintf(result, "%s%s%s", prefix, escaped_username, suffix);
    }
    
    free(escaped_username);
    return result;
}

// Function pointer type for our handlers
typedef char* (*url_handler_t)(const char*);

// A mapping between the URL and the function.
typedef struct {
    const char* prefix;
    url_handler_t handler;
} Route;

// Array of routes to simulate decorator mapping
Route routes[] = {
    {"/hello/", hello},
    {NULL, NULL} // Sentinel to mark the end of the array
};

// Handles an incoming request by routing it to the correct function.
// NOTE: The caller is responsible for freeing the returned memory.
char* handle_request(const char* url) {
    for (int i = 0; routes[i].prefix != NULL; i++) {
        if (strncmp(url, routes[i].prefix, strlen(routes[i].prefix)) == 0) {
            const char* param = url + strlen(routes[i].prefix);
            return routes[i].handler(param);
        }
    }
    
    // Return "404 Not Found" if no route matches
    char* not_found_msg = (char*)malloc(14);
    if (not_found_msg) {
        strcpy(not_found_msg, "404 Not Found");
    }
    return not_found_msg;
}

void run_test_case(const char* test_name, const char* input_url) {
    printf("%s\n", test_name);
    printf("Input: %s\n", input_url);
    char* output = handle_request(input_url);
    if (output) {
        printf("Output: %s\n", output);
        free(output);
    } else {
        printf("Output: Memory allocation failed\n");
    }
    printf("\n");
}

int main() {
    // --- 5 Test Cases ---
    run_test_case("Test Case 1: Simple username", "/hello/Alice");
    run_test_case("Test Case 2: Username with HTML characters", "/hello/<script>alert('XSS')</script>");
    run_test_case("Test Case 3: Empty username", "/hello/");
    run_test_case("Test Case 4: Mismatched URL", "/goodbye/Bob");
    run_test_case("Test Case 5: Username with various special characters", "/hello/Tom & Jerry > \"The Movie\"");
    
    return 0;
}