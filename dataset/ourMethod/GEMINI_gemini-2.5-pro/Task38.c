#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Max size of POST data to accept to prevent resource exhaustion.
#define MAX_POST_SIZE 10240 // 10KB

// Helper function to convert a hex character to its integer value.
static int hex_to_int(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// URL-decodes a string in-place. In-place is safe because the decoded
// string is always shorter than or equal to the encoded string.
// Returns the new length of the modified string.
static size_t url_decode(char *str) {
    if (str == NULL) {
        return 0;
    }
    char *p = str;
    char *q = str;
    while (*p) {
        if (*p == '+') {
            *q++ = ' ';
            p++;
        } else if (*p == '%' && p[1] && p[2] && isxdigit((unsigned char)p[1]) && isxdigit((unsigned char)p[2])) {
            int val1 = hex_to_int(p[1]);
            int val2 = hex_to_int(p[2]);
            *q++ = (char)(val1 * 16 + val2);
            p += 3;
        } else {
            *q++ = *p++;
        }
    }
    *q = '\0';
    return (size_t)(q - str);
}

// Escapes a string for HTML output to prevent XSS.
// Returns a new, dynamically allocated string that must be freed by the caller.
char* html_escape(const char* str) {
    if (str == NULL) {
        return NULL;
    }

    // First pass: calculate required size to avoid buffer overflows.
    size_t new_len = 0;
    for (const char* p = str; *p; p++) {
        switch (*p) {
            case '&': new_len += 5; break; // &amp;
            case '"': new_len += 6; break; // &quot;
            case '\'': new_len += 5; break; // &#39;
            case '<': new_len += 4; break; // &lt;
            case '>': new_len += 4; break; // &gt;
            default: new_len += 1;
        }
    }

    char* escaped = (char*)malloc(new_len + 1);
    if (escaped == NULL) {
        return NULL; // Allocation failed.
    }

    // Second pass: build the new string.
    char* q = escaped;
    for (const char* p = str; *p; p++) {
        switch (*p) {
            case '&':  memcpy(q, "&amp;", 5); q += 5; break;
            case '"':  memcpy(q, "&quot;", 6); q += 6; break;
            case '\'': memcpy(q, "&#39;", 5); q += 5; break;
            case '<':  memcpy(q, "&lt;", 4); q += 4; break;
            case '>':  memcpy(q, "&gt;", 4); q += 4; break;
            default:   *q++ = *p;
        }
    }
    *q = '\0';
    return escaped;
}

// Parses "user_input=value" from form data.
// Returns a new, dynamically allocated string with the value, which must be freed.
char* parse_form_data(const char* data) {
    if (data == NULL) return NULL;
    const char* key = "user_input=";
    const char* p = strstr(data, key);
    if (p == NULL) return NULL;
    
    p += strlen(key); // Move pointer to the beginning of the value.
    
    const char* end = strchr(p, '&');
    size_t len = (end == NULL) ? strlen(p) : (size_t)(end - p);

    char* value = (char*)malloc(len + 1);
    if (value == NULL) return NULL;

    memcpy(value, p, len);
    value[len] = '\0';
    url_decode(value);
    return value;
}

// Prints the HTML form to standard output.
void display_form(void) {
    printf("Content-Type: text/html; charset=utf-8\r\n\r\n");
    printf("<!DOCTYPE html>\n<html lang=\"en\">\n"
           "<head><meta charset=\"UTF-8\"><title>C CGI Form</title></head>\n"
           "<body><h1>Enter some text</h1>\n"
           "<form method=\"POST\" action=\"\">\n"
           "  <input type=\"text\" name=\"user_input\" size=\"50\">\n"
           "  <input type=\"submit\" value=\"Submit\">\n"
           "</form></body></html>\n");
}

// Parses input, escapes it, and prints the response page.
void display_response(const char* input) {
    char* decoded_input = parse_form_data(input);
    if (decoded_input == NULL) {
        display_form(); return;
    }
    char* safe_input = html_escape(decoded_input);
    if (safe_input == NULL) {
        free(decoded_input);
        display_form(); return;
    }
    
    printf("Content-Type: text/html; charset=utf-8\r\n\r\n");
    printf("<!DOCTYPE html>\n<html lang=\"en\">\n"
           "<head><meta charset=\"UTF-8\"><title>C CGI Response</title></head>\n"
           "<body><h1>You entered:</h1>\n"
           "<p>%s</p>\n<a href=\"\">Go back</a></body></html>\n", safe_input);

    free(decoded_input);
    free(safe_input);
}

void run_tests(void) {
    printf("Running tests...\n\n--- html_escape tests ---\n");
    const char* escape_inputs[] = {"hello", "<script>", "&'\"<>", "", "Test '5'"};
    const char* escape_expects[] = {"hello", "&lt;script&gt;", "&amp;&#39;&quot;&lt;&gt;", "", "Test &#39;5&#39;"};
    for (int i = 0; i < 5; ++i) {
        char* result = html_escape(escape_inputs[i]);
        printf("Test %d: %s\n", i + 1, (result && strcmp(result, escape_expects[i]) == 0) ? "PASSED" : "FAILED");
        free(result);
    }

    printf("\n--- parse_form_data tests ---\n");
    const char* parse_inputs[] = {"user_input=hello+world", "user_input=a%%20b", "other=1&user_input=test", "user_input=", "user_input=100%%25+complete"};
    const char* parse_expects[] = {"hello world", "a% b", "test", "", "100% complete"};
    for (int i = 0; i < 5; ++i) {
        char* result = parse_form_data(parse_inputs[i]);
        printf("Test %d: %s\n", i + 1, (result && strcmp(result, parse_expects[i]) == 0) ? "PASSED" : "FAILED");
        free(result);
    }
}

int main(void) {
    if (getenv("REQUEST_METHOD") != NULL) {
        char* request_method = getenv("REQUEST_METHOD");
        if (strcmp(request_method, "POST") == 0) {
            char* content_length_str = getenv("CONTENT_LENGTH");
            if (content_length_str) {
                long content_length = strtol(content_length_str, NULL, 10);
                if (content_length > 0 && content_length <= MAX_POST_SIZE) {
                    char* post_data = (char*)malloc(content_length + 1);
                    if (post_data) {
                        if (fread(post_data, 1, content_length, stdin) == (size_t)content_length) {
                            post_data[content_length] = '\0';
                            display_response(post_data);
                        } else { display_form(); }
                        free(post_data);
                    } else { display_form(); }
                } else { display_form(); }
            } else { display_form(); }
        } else { display_form(); }
    } else {
        run_tests();
    }
    return 0;
}