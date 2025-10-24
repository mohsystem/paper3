
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_RESOURCES 10
#define MAX_STRING_LEN 50

typedef struct {
    char username[MAX_STRING_LEN];
    char password[MAX_STRING_LEN];
    char resources[MAX_RESOURCES][MAX_STRING_LEN];
    int resourceCount;
} User;

typedef struct {
    User users[MAX_USERS];
    int userCount;
} Task125;

void initializeData(Task125* system) {
    system->userCount = 5;
    
    // Admin
    strcpy(system->users[0].username, "admin");
    strcpy(system->users[0].password, "admin123");
    strcpy(system->users[0].resources[0], "Dashboard");
    strcpy(system->users[0].resources[1], "Users");
    strcpy(system->users[0].resources[2], "Settings");
    strcpy(system->users[0].resources[3], "Reports");
    strcpy(system->users[0].resources[4], "Database");
    system->users[0].resourceCount = 5;
    
    // User1
    strcpy(system->users[1].username, "user1");
    strcpy(system->users[1].password, "pass1");
    strcpy(system->users[1].resources[0], "Dashboard");
    strcpy(system->users[1].resources[1], "Profile");
    strcpy(system->users[1].resources[2], "Documents");
    system->users[1].resourceCount = 3;
    
    // User2
    strcpy(system->users[2].username, "user2");
    strcpy(system->users[2].password, "pass2");
    strcpy(system->users[2].resources[0], "Dashboard");
    strcpy(system->users[2].resources[1], "Profile");
    strcpy(system->users[2].resources[2], "Messages");
    system->users[2].resourceCount = 3;
    
    // Guest
    strcpy(system->users[3].username, "guest");
    strcpy(system->users[3].password, "guest123");
    strcpy(system->users[3].resources[0], "Dashboard");
    strcpy(system->users[3].resources[1], "Help");
    system->users[3].resourceCount = 2;
    
    // Manager
    strcpy(system->users[4].username, "manager");
    strcpy(system->users[4].password, "manager456");
    strcpy(system->users[4].resources[0], "Dashboard");
    strcpy(system->users[4].resources[1], "Reports");
    strcpy(system->users[4].resources[2], "Team");
    strcpy(system->users[4].resources[3], "Projects");
    system->users[4].resourceCount = 4;
}

User* findUser(Task125* system, const char* username) {
    for (int i = 0; i < system->userCount; i++) {
        if (strcmp(system->users[i].username, username) == 0) {
            return &system->users[i];
        }
    }
    return NULL;
}

bool authenticate(Task125* system, const char* username, const char* password) {
    User* user = findUser(system, username);
    return user != NULL && strcmp(user->password, password) == 0;
}

bool canAccessResource(Task125* system, const char* username, const char* resource) {
    User* user = findUser(system, username);
    if (user == NULL) return false;
    
    for (int i = 0; i < user->resourceCount; i++) {
        if (strcmp(user->resources[i], resource) == 0) {
            return true;
        }
    }
    return false;
}

void accessResource(Task125* system, const char* username, const char* password, 
                   const char* resource, char* result) {
    if (!authenticate(system, username, password)) {
        strcpy(result, "Authentication failed: Invalid username or password");
        return;
    }
    
    if (canAccessResource(system, username, resource)) {
        sprintf(result, "Access granted: %s can access %s", username, resource);
    } else {
        sprintf(result, "Access denied: %s cannot access %s", username, resource);
    }
}

int main() {
    Task125 system;
    initializeData(&system);
    char result[200];
    
    printf("Test Case 1: Admin accessing Dashboard\\n");
    accessResource(&system, "admin", "admin123", "Dashboard", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 2: User1 accessing Documents\\n");
    accessResource(&system, "user1", "pass1", "Documents", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 3: Guest trying to access Database\\n");
    accessResource(&system, "guest", "guest123", "Database", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 4: Invalid credentials\\n");
    accessResource(&system, "user1", "wrongpass", "Dashboard", result);
    printf("%s\\n\\n", result);
    
    printf("Test Case 5: Manager accessing Reports\\n");
    accessResource(&system, "manager", "manager456", "Reports", result);
    printf("%s\\n\\n", result);
    
    printf("Bonus: Listing all accessible resources for user1\\n");
    User* user = findUser(&system, "user1");
    if (user != NULL) {
        printf("user1 can access: ");
        for (int i = 0; i < user->resourceCount; i++) {
            printf("%s", user->resources[i]);
            if (i < user->resourceCount - 1) printf(", ");
        }
        printf("\\n");
    }
    
    return 0;
}
