
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char username[100];
    char email[100];
    char ssn[20];
} Session;

Session session;

void setSession(const char* key, const char* value) {
    if (strcmp(key, "username") == 0) {
        strcpy(session.username, value);
    } else if (strcmp(key, "email") == 0) {
        strcpy(session.email, value);
    } else if (strcmp(key, "ssn") == 0) {
        strcpy(session.ssn, value);
    }
}

char* info() {
    static char result[300];
    sprintf(result, "Username: %s, Email: %s, SSN: %s", 
            session.username, session.email, session.ssn);
    return result;
}

int main() {
    // Test Case 1
    setSession("username", "john_doe");
    setSession("email", "john@example.com");
    setSession("ssn", "123-45-6789");
    printf("Test Case 1: %s\\n", info());
    
    // Test Case 2
    setSession("username", "jane_smith");
    setSession("email", "jane@example.com");
    setSession("ssn", "987-65-4321");
    printf("Test Case 2: %s\\n", info());
    
    // Test Case 3
    setSession("username", "bob_jones");
    setSession("email", "bob@example.com");
    setSession("ssn", "111-22-3333");
    printf("Test Case 3: %s\\n", info());
    
    // Test Case 4
    setSession("username", "alice_wonder");
    setSession("email", "alice@example.com");
    setSession("ssn", "444-55-6666");
    printf("Test Case 4: %s\\n", info());
    
    // Test Case 5
    setSession("username", "charlie_brown");
    setSession("email", "charlie@example.com");
    setSession("ssn", "777-88-9999");
    printf("Test Case 5: %s\\n", info());
    
    return 0;
}
