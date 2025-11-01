
// Pure C does not have built-in web server capabilities
// Below is a conceptual implementation showing the core logic

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char username[100];
    char email[100];
    char ssn[20];
} Session;

void getLastFourDigits(const char* ssn, char* result) {
    int len = strlen(ssn);
    if (len >= 4) {
        strcpy(result, ssn + len - 4);
    } else {
        strcpy(result, "N/A");
    }
}

void info(Session* session, char* output) {
    char last4[5] = {0};
    getLastFourDigits(session->ssn, last4);
    
    sprintf(output, 
            "<html><body>"
            "<h1>User Information</h1>"
            "<p>Username: %s</p>"
            "<p>Email: %s</p>"
            "<p>Last 4 digits of SSN: %s</p>"
            "</body></html>",
            session->username,
            session->email,
            last4);
}

int main() {
    char output[1000];
    
    // Test case 1
    Session session1 = {"john_doe", "john@example.com", "123456789"};
    info(&session1, output);
    printf("Test Case 1:\\n%s\\n\\n", output);
    
    // Test case 2
    Session session2 = {"jane_smith", "jane@example.com", "987654321"};
    info(&session2, output);
    printf("Test Case 2:\\n%s\\n\\n", output);
    
    // Test case 3
    Session session3 = {"bob_jones", "bob@example.com", "555443333"};
    info(&session3, output);
    printf("Test Case 3:\\n%s\\n\\n", output);
    
    // Test case 4
    Session session4 = {"alice_wilson", "alice@example.com", "111223333"};
    info(&session4, output);
    printf("Test Case 4:\\n%s\\n\\n", output);
    
    // Test case 5
    Session session5 = {"charlie_brown", "charlie@example.com", "999887777"};
    info(&session5, output);
    printf("Test Case 5:\\n%s\\n\\n", output);
    
    return 0;
}
