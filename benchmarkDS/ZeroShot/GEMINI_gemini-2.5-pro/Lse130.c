#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: This C example does not run a live web server.
// It simulates handling a request by processing a request URI string,
// focusing on the core logic to keep the code self-contained and simple.

/**
 * @brief Performs HTML entity escaping on a string.
 * This is a crucial security measure to prevent Cross-Site Scripting (XSS).
 * The caller is responsible for freeing the returned memory.
 *
 * @param input The string to escape.
 * @return A new, dynamically allocated escaped string.
 */
char* secureHtmlEscape(const char* input) {
    if (!input) return NULL;

    // Allocate memory generously, as escaping increases size.
    // Each char can become up to 6 chars long (e.g., '&' -> "&amp;").
    size_t input_len = strlen(input);
    char* escaped = (char*)malloc(input_len * 6 + 1);
    if (!escaped) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    char* p_out = escaped;
    const char* p_in = input;
    while (*p_in) {
        switch (*p_in) {
            case '&':  p_out += sprintf(p_out, "&amp;");   break;
            case '<':  p_out += sprintf(p_out, "&lt;");    break;
            case '>':  p_out += sprintf(p_out, "&gt;");    break;
            case '"':  p_out += sprintf(p_out, "&quot;");  break;
            case '\'': p_out += sprintf(p_out, "&#x27;");  break;
            default:   *p_out++ = *p_in;                  break;
        }
        p_in++;
    }
    *p_out = '\0';
    return escaped;
}

/**
 * @brief Renders a response based on the request "environment".
 * In this simulation, the environment is just the request URI string.
 * The caller is responsible for freeing the returned memory.
 *
 * @param request_uri The URI of the request, e.g., "/?name=Alice".
 * @return A string containing the full HTTP response, or NULL on error.
 */
char* render_response_from_env(const char* request_uri) {
    // Get the name parameter from the request URI.
    char name[256] = "World"; // Buffer for the name parameter
    const char* query_start = strchr(request_uri, '?');
    if (query_start) {
        const char* name_param = strstr(query_start + 1, "name=");
        if (name_param) {
            const char* value_start = name_param + 5;
            const char* value_end = strchr(value_start, '&');
            size_t len = (value_end) ? (size_t)(value_end - value_start) : strlen(value_start);
            if (len > 0 && len < sizeof(name)) {
                strncpy(name, value_start, len);
                name[len] = '\0';
            }
        }
    }

    // Get the template from the "environment" (hardcoded here).
    const char* template_part1 = "<html><body><h1>Hello, ";
    const char* template_part2 = "!</h1></body></html>";

    // Securely render the template.
    char* escaped_name = secureHtmlEscape(name);
    if (!escaped_name) return NULL;

    // Calculate buffer size and allocate memory for the response body.
    size_t body_len = strlen(template_part1) + strlen(escaped_name) + strlen(template_part2);
    char* body = (char*)malloc(body_len + 1);
    if (!body) {
        free(escaped_name);
        perror("Failed to allocate memory for response body");
        return NULL;
    }
    sprintf(body, "%s%s%s", template_part1, escaped_name, template_part2);
    free(escaped_name);

    // Calculate buffer size and allocate memory for the full HTTP response.
    // (HTTP header length is approx 80 chars)
    size_t response_len = strlen(body) + 80;
    char* response = (char*)malloc(response_len);
    if (!response) {
        free(body);
        perror("Failed to allocate memory for full response");
        return NULL;
    }

    // Construct the full HTTP response.
    snprintf(response, response_len,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Content-Length: %zu\r\n"
             "\r\n"
             "%s",
             strlen(body), body);
    
    free(body);
    return response;
}

/**
 * @brief Home function that calls the render function.
 * The caller is responsible for freeing the returned memory.
 *
 * @param request_uri The request URI.
 * @return The full HTTP response string.
 */
char* home(const char* request_uri) {
    return render_response_from_env(request_uri);
}

void run_test_case(int case_num, const char* test_uri) {
    printf("--- Test Case %d ---\n", case_num);
    printf("Input URI: %s\n", test_uri);
    char* response = home(test_uri);
    if (response) {
        printf("Generated Response:\n%s\n\n", response);
        free(response); // Clean up memory
    } else {
        printf("Failed to generate response.\n\n");
    }
}

int main() {
    // In lieu of a web server, the main method runs 5 test cases directly.
    run_test_case(1, "/?name=Alice");
    run_test_case(2, "/");
    run_test_case(3, "/?name=");
    run_test_case(4, "/?name=Bob & Charlie");
    run_test_case(5, "/?name=<script>alert('XSS')</script>");

    return 0;
}