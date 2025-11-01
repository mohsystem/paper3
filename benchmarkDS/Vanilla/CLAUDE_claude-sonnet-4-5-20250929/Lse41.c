
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 10
#define MAX_STRING 100

typedef struct {
    char username[MAX_STRING];
    char email[MAX_STRING];
    char ssn[MAX_STRING];
} UserInfo;

typedef struct {
    char userId[MAX_STRING];
    UserInfo info;
} UserEntry;

UserEntry userDatabase[MAX_USERS];
int userCount = 0;

void getLast4SSN(const char* ssn, char* result) {
    int len = strlen(ssn);
    if (len >= 4) {
        strcpy(result, ssn + len - 4);
    } else {
        strcpy(result, "****");
    }
}

void getInfoPage(const char* userId, char* response) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].userId, userId) == 0) {
            UserInfo user = userDatabase[i].info;
            char last4[5];
            getLast4SSN(user.ssn, last4);
            
            sprintf(response, "User Information\\n================\\nUsername: %s\\nEmail: %s\\nSSN (Last 4): %s\\n",
                    user.username, user.email, last4);
            return;
        }
    }
    strcpy(response, "User not found");
}

void addUser(const char* userId, const char* username, const char* email, const char* ssn) {
    strcpy(userDatabase[userCount].userId, userId);
    strcpy(userDatabase[userCount].info.username, username);
    strcpy(userDatabase[userCount].info.email, email);
    strcpy(userDatabase[userCount].info.ssn, ssn);
    userCount++;
}

int main() {
    char response[500];
    
    // Initialize test data
    addUser("user1", "john_doe", "john@example.com", "123456789");
    addUser("user2", "jane_smith", "jane@example.com", "987654321");
    addUser("user3", "bob_wilson", "bob@example.com", "555443333");
    addUser("user4", "alice_brown", "alice@example.com", "111222333");
    addUser("user5", "charlie_davis", "charlie@example.com", "999888777");
    
    // Test cases
    printf("Test Case 1:\\n");
    getInfoPage("user1", response);
    printf("%s\\n", response);
    
    printf("Test Case 2:\\n");
    getInfoPage("user2", response);
    printf("%s\\n", response);
    
    printf("Test Case 3:\\n");
    getInfoPage("user3", response);
    printf("%s\\n", response);
    
    printf("Test Case 4:\\n");
    getInfoPage("user4", response);
    printf("%s\\n", response);
    
    printf("Test Case 5:\\n");
    getInfoPage("user5", response);
    printf("%s\\n", response);
    
    return 0;
}
