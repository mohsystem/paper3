#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Note: Creating an HTTP server/API endpoint in C without external libraries
// (like libmicrohttpd, mongoose, etc.) is a very complex task involving low-level
// socket programming (which is platform-specific) and manual HTTP parsing.
//
// This program simulates the back-end logic of an API: it takes a string
// representing user data, parses it, and stores it in a dynamic, in-memory "database".

// User structure
typedef struct {
    char* name;
    char* email;
} User;

// In-memory "database"
User** userDatabase = NULL;
int userCount = 0;
int dbCapacity = 0;

// Function to simulate storing user data from a request body
// The body is expected in a simple format: "name,email"
void storeUserData(const char* requestBody) {
    printf("Processing request body: \"%s\"\n", requestBody);

    // Make a mutable copy of the request body for strtok
    char* bodyCopy = (char*)malloc(strlen(requestBody) + 1);
    if (bodyCopy == NULL) {
        fprintf(stderr, "Failed to allocate memory for body copy.\n");
        return;
    }
    strcpy(bodyCopy, requestBody);

    // Parse the string
    char* name_str = strtok(bodyCopy, ",");
    char* email_str = strtok(NULL, ",");

    if (name_str == NULL || email_str == NULL) {
        fprintf(stderr, "Error: Invalid request body format.\n");
        free(bodyCopy);
        return;
    }
    
    // Allocate memory for the new user
    User* newUser = (User*)malloc(sizeof(User));
    if (newUser == NULL) {
        fprintf(stderr, "Failed to allocate memory for new user.\n");
        free(bodyCopy);
        return;
    }
    newUser->name = (char*)malloc(strlen(name_str) + 1);
    newUser->email = (char*)malloc(strlen(email_str) + 1);

    if (newUser->name == NULL || newUser->email == NULL) {
        fprintf(stderr, "Failed to allocate memory for user fields.\n");
        free(newUser->name);
        free(newUser->email);
        free(newUser);
        free(bodyCopy);
        return;
    }

    strcpy(newUser->name, name_str);
    strcpy(newUser->email, email_str);

    // Add user to the database, resizing if necessary
    if (userCount >= dbCapacity) {
        dbCapacity = (dbCapacity == 0) ? 4 : dbCapacity * 2;
        User** newDb = (User**)realloc(userDatabase, dbCapacity * sizeof(User*));
        if (newDb == NULL) {
            fprintf(stderr, "Failed to reallocate database memory.\n");
            // Free the new user's memory as it cannot be stored
            free(newUser->name);
            free(newUser->email);
            free(newUser);
            free(bodyCopy);
            return;
        }
        userDatabase = newDb;
    }

    userDatabase[userCount++] = newUser;
    printf("Successfully stored user: %s\n", newUser->name);
    free(bodyCopy);
}

void printDatabase() {
    printf("\n--- Current Database State ---\n");
    if (userCount == 0) {
        printf("Database is empty.\n");
    } else {
        for (int i = 0; i < userCount; i++) {
            printf("User %d: Name = %s, Email = %s\n", i + 1, userDatabase[i]->name, userDatabase[i]->email);
        }
    }
    printf("----------------------------\n");
}

void cleanupDatabase() {
    for (int i = 0; i < userCount; i++) {
        free(userDatabase[i]->name);
        free(userDatabase[i]->email);
        free(userDatabase[i]);
    }
    free(userDatabase);
    userDatabase = NULL;
    userCount = 0;
    dbCapacity = 0;
}

int main() {
    printf("Simulating API endpoint data storage in C.\n");

    // --- 5 Test Cases ---
    printf("\nRunning 5 test cases...\n\n");
    storeUserData("Alice,alice@example.com");
    storeUserData("Bob,bob@example.com");
    storeUserData("Charlie,charlie@example.com");
    storeUserData("David,david@example.com");
    storeUserData("Eve,eve@example.com");

    // Verify the contents of the database
    printDatabase();

    // Clean up all allocated memory
    cleanupDatabase();
    printf("Database cleaned up.\n");

    return 0;
}