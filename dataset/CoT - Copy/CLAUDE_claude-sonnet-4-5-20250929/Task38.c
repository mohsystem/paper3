
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_INPUT 1000
#define MAX_TOKEN 64
#define MAX_NAME 100
#define MAX_EMAIL 100
#define MAX_MESSAGE 500

typedef struct {
    char token[MAX_TOKEN];
    int valid;
} CSRFToken;

CSRFToken csrf_tokens[10];
int token_count = 0;

void generate_csrf_token(char* token) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    srand(time(NULL));
    for (int i = 0; i < 32; i++) {
        token[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    token[32] = '\\0';
}

void escape_html(const char* input, char* output, size_t max_len) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < max_len - 6; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&output[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&output[j], "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(&output[j], "&#x2F;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\\0';
}

void sanitize_input(char* input, size_t max_length) {
    size_t len = strlen(input);
    while (len > 0 && isspace(input[len - 1])) {
        input[--len] = '\\0';
    }
    
    size_t start = 0;
    while (input[start] && isspace(input[start])) {
        start++;
    }
    
    if (start > 0) {
        memmove(input, input + start, len - start + 1);
    }
    
    if (strlen(input) > max_length) {
        input[max_length] = '\\0';
    }
}

int is_valid_email(const char* email) {
    int at_count = 0;
    int dot_after_at = 0;
    int at_pos = -1;
    
    for (int i = 0; email[i]; i++) {
        if (email[i] == '@') {
            at_count++;
            at_pos = i;
        }
        if (at_pos >= 0 && email[i] == '.') {
            dot_after_at = 1;
        }
    }
    
    return (at_count == 1 && dot_after_at && at_pos > 0 && strlen(email) > at_pos + 2);
}

void get_form_page(char* output, const char* csrf_token) {
    char escaped_token[MAX_TOKEN * 6];
    escape_html(csrf_token, escaped_token, sizeof(escaped_token));
    
    sprintf(output,
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<title>Secure Form</title></head><body>"
        "<h1>User Input Form</h1>"
        "<form method='POST' action='/submit'>"
        "<input type='hidden' name='csrf_token' value='%s'>"
        "<label>Name: <input type='text' name='name' maxlength='100' required></label><br>"
        "<label>Email: <input type='email' name='email' maxlength='100' required></label><br>"
        "<label>Message: <textarea name='message' maxlength='500' required></textarea></label><br>"
        "<input type='submit' value='Submit'>"
        "</form></body></html>",
        escaped_token);
}

void process_form_submission(const char* name, const char* email, const char* message, char* output) {
    char escaped_name[MAX_NAME * 6];
    char escaped_email[MAX_EMAIL * 6];
    char escaped_message[MAX_MESSAGE * 6];
    
    escape_html(name, escaped_name, sizeof(escaped_name));
    escape_html(email, escaped_email, sizeof(escaped_email));
    escape_html(message, escaped_message, sizeof(escaped_message));
    
    sprintf(output,
        "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
        "<title>Form Submission Result</title></head><body>"
        "<h1>Your Submitted Data</h1>"
        "<p><strong>Name:</strong> %s</p>"
        "<p><strong>Email:</strong> %s</p>"
        "<p><strong>Message:</strong> %s</p>"
        "<a href='/'>Back to Form</a></body></html>",
        escaped_name, escaped_email, escaped_message);
}

int main() {
    char output[4096];
    char csrf_token[MAX_TOKEN];
    
    printf("Test 1: Generate form page\\n");
    generate_csrf_token(csrf_token);
    get_form_page(output, csrf_token);
    printf("%.80s...\\n\\n", output);
    
    printf("Test 2: Process valid submission\\n");
    char name[] = "John Doe";
    char email[] = "john@example.com";
    char message[] = "Hello World";
    sanitize_input(name, MAX_NAME);
    sanitize_input(email, MAX_EMAIL);
    sanitize_input(message, MAX_MESSAGE);
    if (is_valid_email(email)) {
        process_form_submission(name, email, message, output);
        printf("%.100s...\\n\\n", output);
    }
    
    printf("Test 3: Test XSS prevention\\n");
    char xss_name[] = "<script>alert(1)</script>";
    char valid_email[] = "test@test.com";
    char test_msg[] = "Test message";
    sanitize_input(xss_name, MAX_NAME);
    process_form_submission(xss_name, valid_email, test_msg, output);
    printf("%.100s...\\n\\n", output);
    
    printf("Test 4: Invalid email format\\n");
    char test_name[] = "Test User";
    char invalid_email[] = "invalid-email";
    if (!is_valid_email(invalid_email)) {
        printf("Error: Invalid email format\\n\\n");
    }
    
    printf("Test 5: Input sanitization\\n");
    char padded_input[] = "  Test Input  ";
    sanitize_input(padded_input, MAX_NAME);
    printf("Sanitized: '%s'\\n", padded_input);
    
    return 0;
}
