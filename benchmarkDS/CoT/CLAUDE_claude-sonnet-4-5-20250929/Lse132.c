
// C - Basic implementation (no web framework, demonstrates concept)
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_HTML_LENGTH 1024

bool is_valid_username(const char* username) {
    if (username == NULL || strlen(username) == 0 || strlen(username) > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    for (int i = 0; username[i] != '\\0'; i++) {
        if (!isalnum(username[i]) && username[i] != '_' && username[i] != '-') {
            return false;
        }
    }
    return true;
}

void html_escape(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < output_size - 10; i++) {
        switch (input[i]) {
            case '&':
                j += snprintf(output + j, output_size - j, "&amp;");
                break;
            case '<':
                j += snprintf(output + j, output_size - j, "&lt;");
                break;
            case '>':
                j += snprintf(output + j, output_size - j, "&gt;");
                break;
            case '"':
                j += snprintf(output + j, output_size - j, "&quot;");
                break;
            case '\\'':
                j += snprintf(output + j, output_size - j, "&#x27;");
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\\0';
}

void render_template(const char* username, const char* error, char* html_output, size_t output_size) {
    snprintf(html_output, output_size,
        "<!DOCTYPE html><html><head><title>Hello</title></head><body>"
        "<h1>Hello, %s!</h1>%s%s%s</body></html>",
        username,
        error[0] ? "<p style=\\"color: red;\\">" : "",
        error,
        error[0] ? "</p>" : ""
    );
}

void hello(const char* username, char* response, size_t response_size) {
    char sanitized[MAX_USERNAME_LENGTH * 6 + 1];
    char error[256] = "";
    
    if (!is_valid_username(username)) {
        strcpy(sanitized, "Invalid User");
        strcpy(error, "Username must be alphanumeric (1-50 chars)");
    } else {
        html_escape(username, sanitized, sizeof(sanitized));
    }
    
    render_template(sanitized, error, response, response_size);
}

int main() {
    printf("Test cases for /hello/<username>:\\n");
    
    const char* test_cases[] = {
        "john",
        "jane_doe",
        "user-123",
        "<script>alert('xss')</script>",
        "verylongusernamethatshouldbevalidatedproperly"
    };
    
    char response[MAX_HTML_LENGTH];
    
    for (int i = 0; i < 5; i++) {
        printf("%d. /hello/%s\\n", i + 1, test_cases[i]);
        hello(test_cases[i], response, sizeof(response));
        printf("   Response length: %zu bytes\\n", strlen(response));
    }
    
    return 0;
}
