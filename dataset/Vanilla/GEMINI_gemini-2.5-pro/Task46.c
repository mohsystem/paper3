#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// User structure
typedef struct {
    char* name;
    char* email;
    char* password; // In a real app, this should be hashed
} User;

// Database structure to hold users
typedef struct {
    User** users;
    int count;
    int capacity;
} Database;

/**
 * @brief Registers a new user in the database.
 * 
 * @param db Pointer to the database structure.
 * @param name The user's name.
 * @param email The user's email (must be unique).
 * @param password The user's password.
 * @return 1 on success, 0 on failure (e.g., duplicate email or memory allocation error).
 */
int registerUser(Database* db, const char* name, const char* email, const char* password) {
    // Check for duplicate email
    for (int i = 0; i < db->count; ++i) {
        if (strcmp(db->users[i]->email, email) == 0) {
            printf("Registration failed: Email '%s' already exists.\n", email);
            return 0; // Failure
        }
    }

    // Resize database if full
    if (db->count >= db->capacity) {
        int new_capacity = db->capacity == 0 ? 10 : db->capacity * 2;
        User** new_users = (User**)realloc(db->users, new_capacity * sizeof(User*));
        if (new_users == NULL) {
            printf("Memory allocation failed!\n");
            return 0; // Failure
        }
        db->users = new_users;
        db->capacity = new_capacity;
    }

    // Create new user
    User* newUser = (User*)malloc(sizeof(User));
    if (newUser == NULL) return 0;

    newUser->name = (char*)malloc(strlen(name) + 1);
    newUser->email = (char*)malloc(strlen(email) + 1);
    newUser->password = (char*)malloc(strlen(password) + 1);

    if (newUser->name == NULL || newUser->email == NULL || newUser->password == NULL) {
        free(newUser->name);
        free(newUser->email);
        free(newUser->password);
        free(newUser);
        return 0; // Memory allocation failed
    }

    strcpy(newUser->name, name);
    strcpy(newUser->email, email);
    strcpy(newUser->password, password);

    // Add user to database
    db->users[db->count] = newUser;
    db->count++;

    printf("Registration successful for: User{name='%s', email='%s'}\n", name, email);
    return 1; // Success
}

void freeDatabase(Database* db) {
    if (db == NULL) return;
    for (int i = 0; i < db->count; i++) {
        free(db->users[i]->name);
        free(db->users[i]->email);
        free(db->users[i]->password);
        free(db->users[i]);
    }
    free(db->users);
}

int main() {
    Database db = { NULL, 0, 0 };

    printf("--- User Registration System ---\n");

    // Test Case 1: Successful registration
    printf("\nTest Case 1:\n");
    registerUser(&db, "Alice", "alice@example.com", "password123");

    // Test Case 2: Successful registration
    printf("\nTest Case 2:\n");
    registerUser(&db, "Bob", "bob@example.com", "securepass");

    // Test Case 3: Failed registration (duplicate email)
    printf("\nTest Case 3:\n");
    registerUser(&db, "Alice Smith", "alice@example.com", "anotherpass");

    // Test Case 4: Successful registration
    printf("\nTest Case 4:\n");
    registerUser(&db, "Charlie", "charlie@example.com", "charlie!@#");

    // Test Case 5: Successful registration
    printf("\nTest Case 5:\n");
    registerUser(&db, "Diana", "diana@example.com", "diana_pass");

    printf("\n--- Current Database State ---\n");
    for (int i = 0; i < db.count; ++i) {
        printf("Email: %s, User: {name='%s'}\n", db.users[i]->email, db.users[i]->name);
    }

    // Clean up all allocated memory
    freeDatabase(&db);

    return 0;
}