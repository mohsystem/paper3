
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Simulate database connection
int checkEmailExists(const char* email) {
    // Mock database check
    if (strcmp(email, "user1@example.com") == 0 || 
        strcmp(email, "user2@example.com") == 0) {
        return 1;
    }
    return 0;
}

int callUnsubscribeScript(const char* email) {
    // Simulate calling an unsubscribe script
    // In real scenario, this would execute an external script or perform database update
    if (email != NULL && strchr(email, '@') != NULL) {
        return 0; // Success
    }
    return 1; // Failure
}

int unsubscribeEmail(const char* email) {
    if (email == NULL) {
        return 0;
    }
    
    // Check if email exists in database
    int emailExists = checkEmailExists(email);
    
    if (emailExists) {
        // Email exists, call unsubscribe script
        int scriptResult = callUnsubscribeScript(email);
        
        if (scriptResult == 0) {
            return 1; // Success
        } else {
            return 0; // Script failed
        }
    } else {
        // Email doesn't exist
        return 0;
    }
}

int main() {
    // Test cases
    printf("Test Case 1: %d\\n", unsubscribeEmail("user1@example.com"));
    printf("Test Case 2: %d\\n", unsubscribeEmail("user2@example.com"));
    printf("Test Case 3: %d\\n", unsubscribeEmail("nonexistent@example.com"));
    printf("Test Case 4: %d\\n", unsubscribeEmail("invalid-email"));
    printf("Test Case 5: %d\\n", unsubscribeEmail("user5@example.com"));
    
    return 0;
}
