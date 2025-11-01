
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 256

typedef struct {
    char username[MAX_LEN];
    char email[MAX_LEN];
    char ssn[MAX_LEN];
} Session;

void escape_html(const char* input, char* output, size_t output_size) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < output_size - 6; i++) {
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
            default:
                output[j++] = input[i];
                break;
        }
    }
    output[j] = '\\0';
}

void get_last_four_digits(const char* ssn, char* output) {
    size_t len = strlen(ssn);
    if (len < 4) {
        strcpy(output, "****");
    } else {
        strcpy(output, ssn + len - 4);
    }
}

char* info(Session* session) {
    static char result[2048];
    char username_safe[MAX_LEN * 6];
    char email_safe[MAX_LEN * 6];
    char ssn_last4[5];
    char ssn_last4_safe[30];
    
    if (strlen(session->username) == 0 && strlen(session->email) == 0 && strlen(session->ssn) == 0) {
        strcpy(result, "<html><body><h1>Error: No session data found</h1></body></html>");
        return result;
    }
    
    escape_html(session->username, username_safe, sizeof(username_safe));
    escape_html(session->email, email_safe, sizeof(email_safe));
    get_last_four_digits(session->ssn, ssn_last4);
    escape_html(ssn_last4, ssn_last4_safe, sizeof(ssn_last4_safe));
    
    snprintf(result, sizeof(result),
             "<html><head><title>User Info</title></head><body>"
             "<h1>User Information</h1>"
             "<p><strong>Username:</strong> %s</p>"
             "<p><strong>Email:</strong> %s</p>"
             "<p><strong>SSN (Last 4):</strong> %s</p>"
             "</body></html>",
             username_safe, email_safe, ssn_last4_safe);
    
    return result;
}

int main() {
    Session session;
    
    // Test case 1: Normal data
    printf("Test 1:\\n");
    strcpy(session.username, "john_doe");
    strcpy(session.email, "john@example.com");
    strcpy(session.ssn, "123456789");
    printf("%s\\n\\n", info(&session));
    
    // Test case 2: XSS attempt
    printf("Test 2 (XSS):\\n");
    strcpy(session.username, "jane<script>alert('xss')</script>");
    strcpy(session.email, "jane@test.com");
    strcpy(session.ssn, "987654321");
    printf("%s\\n\\n", info(&session));
    
    // Test case 3: Short SSN
    printf("Test 3 (Short SSN):\\n");
    strcpy(session.username, "bob");
    strcpy(session.email, "bob@mail.com");
    strcpy(session.ssn, "111");
    printf("%s\\n\\n", info(&session));
    
    // Test case 4: Normal data
    printf("Test 4:\\n");
    strcpy(session.username, "alice");
    strcpy(session.email, "alice@domain.com");
    strcpy(session.ssn, "555667788");
    printf("%s\\n\\n", info(&session));
    
    // Test case 5: Empty session
    printf("Test 5 (Empty):\\n");
    memset(&session, 0, sizeof(Session));
    printf("%s\\n", info(&session));
    
    return 0;
}
