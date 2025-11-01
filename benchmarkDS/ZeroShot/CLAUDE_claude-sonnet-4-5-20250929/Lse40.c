
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_STRING_LEN 256

typedef struct {
    char username[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
    char ssn[MAX_STRING_LEN];
} Session;

void getLast4Digits(const char* ssn, char* result) {
    int len = strlen(ssn);
    if (len >= 4) {
        strcpy(result, ssn + len - 4);
    } else {
        strcpy(result, "N/A");
    }
}

void info(Session* session, char* output) {
    char username[MAX_STRING_LEN];
    char email[MAX_STRING_LEN];
    char last4SSN[5];
    
    if (strlen(session->username) > 0) {
        strcpy(username, session->username);
    } else {
        strcpy(username, "N/A");
    }
    
    if (strlen(session->email) > 0) {
        strcpy(email, session->email);
    } else {
        strcpy(email, "N/A");
    }
    
    getLast4Digits(session->ssn, last4SSN);
    
    sprintf(output, 
            "<html><body>\\n"
            "<h1>User Information</h1>\\n"
            "<p>Username: %s</p>\\n"
            "<p>Email: %s</p>\\n"
            "<p>Last 4 digits of SSN: %s</p>\\n"
            "</body></html>",
            username, email, last4SSN);
}

int main() {
    char output[1024];
    Session session;
    
    // Test Case 1
    printf("Test Case 1:\\n");
    strcpy(session.username, "john_doe");
    strcpy(session.email, "john@example.com");
    strcpy(session.ssn, "123456789");
    info(&session, output);
    printf("%s\\n\\n", output);
    
    // Test Case 2
    printf("Test Case 2:\\n");
    strcpy(session.username, "jane_smith");
    strcpy(session.email, "jane@example.com");
    strcpy(session.ssn, "987654321");
    info(&session, output);
    printf("%s\\n\\n", output);
    
    // Test Case 3
    printf("Test Case 3:\\n");
    strcpy(session.username, "bob_wilson");
    strcpy(session.email, "bob@example.com");
    strcpy(session.ssn, "555443333");
    info(&session, output);
    printf("%s\\n\\n", output);
    
    // Test Case 4 (Empty session)
    printf("Test Case 4 (Empty session):\\n");
    strcpy(session.username, "");
    strcpy(session.email, "");
    strcpy(session.ssn, "");
    info(&session, output);
    printf("%s\\n\\n", output);
    
    // Test Case 5 (Short SSN)
    printf("Test Case 5 (Short SSN):\\n");
    strcpy(session.username, "alice");
    strcpy(session.email, "alice@test.com");
    strcpy(session.ssn, "12");
    info(&session, output);
    printf("%s\\n", output);
    
    return 0;
}
