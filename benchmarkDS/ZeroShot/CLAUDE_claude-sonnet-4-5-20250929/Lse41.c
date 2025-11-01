
// WARNING: This is a simplified demonstration. Real web applications require proper frameworks.
// This code demonstrates INSECURE practices for educational purposes only
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char username[50];
    char email[100];
    char ssn[10];  // INSECURE: Never store SSN like this!
} User;

// Mock database - INSECURE
User getCurrentUser() {
    User user;
    strcpy(user.username, "john_doe");
    strcpy(user.email, "john@example.com");
    strcpy(user.ssn, "123456789");  // INSECURE: Hardcoded sensitive data
    return user;
}

void handleInfoRequest() {
    User user = getCurrentUser();
    char lastFour[5];
    
    // Get last 4 digits of SSN
    strncpy(lastFour, &user.ssn[5], 4);
    lastFour[4] = '\\0';
    
    printf("<html><body>\\n");
    printf("<h1>User Information</h1>\\n");
    printf("<p>Username: %s</p>\\n", user.username);
    printf("<p>Email: %s</p>\\n", user.email);
    printf("<p>SSN (Last 4): ***-**-%s</p>\\n", lastFour);
    printf("</body></html>\\n");
}

void runTestCases() {
    printf("Test Cases:\\n");
    
    User users[5] = {
        {"john_doe", "john@example.com", "123456789"},
        {"jane_smith", "jane@example.com", "987654321"},
        {"bob_jones", "bob@example.com", "555443333"},
        {"alice_wonder", "alice@example.com", "111223333"},
        {"charlie_brown", "charlie@example.com", "999887777"}
    };
    
    for (int i = 0; i < 5; i++) {
        char lastFour[5];
        strncpy(lastFour, &users[i].ssn[5], 4);
        lastFour[4] = '\\0';
        
        printf("%d. User: %s\\n", i + 1, users[i].username);
        printf("   Email: %s\\n", users[i].email);
        printf("   Last 4 SSN: %s\\n", lastFour);
    }
}

int main() {
    runTestCases();
    return 0;
}
