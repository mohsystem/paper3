
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_USERNAME 50
#define MAX_PASSWORD 50
#define MAX_USERS 10

typedef struct {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    bool isAdmin;
} User;

typedef struct {
    User* currentUser;
} Session;

typedef struct {
    User users[MAX_USERS];
    int count;
} UserDatabase;

bool checkPassword(const User* user, const char* password) {
    return strcmp(user->password, password) == 0;
}

void initSession(Session* session) {
    session->currentUser = NULL;
}

void login(Session* session, User* user) {
    session->currentUser = user;
}

void logout(Session* session) {
    session->currentUser = NULL;
}

bool isLoggedIn(const Session* session) {
    return session->currentUser != NULL;
}

const char* redirectUser(const Session* session) {
    if (!isLoggedIn(session)) {
        return "Redirect to: /login";
    }

    if (session->currentUser->isAdmin) {
        return "Redirect to: /admin";
    } else {
        return "Redirect to: /user";
    }
}

User* findUser(UserDatabase* db, const char* username) {
    for (int i = 0; i < db->count; i++) {
        if (strcmp(db->users[i].username, username) == 0) {
            return &db->users[i];
        }
    }
    return NULL;
}

bool authenticate(const char* username, const char* password, UserDatabase* db) {
    User* user = findUser(db, username);
    if (user != NULL && checkPassword(user, password)) {
        return true;
    }
    return false;
}

void addUser(UserDatabase* db, const char* username, const char* password, bool isAdmin) {
    if (db->count < MAX_USERS) {
        strncpy(db->users[db->count].username, username, MAX_USERNAME - 1);
        db->users[db->count].username[MAX_USERNAME - 1] = '\\0';
        strncpy(db->users[db->count].password, password, MAX_PASSWORD - 1);
        db->users[db->count].password[MAX_PASSWORD - 1] = '\\0';
        db->users[db->count].isAdmin = isAdmin;
        db->count++;
    }
}

int main() {
    UserDatabase db;
    db.count = 0;

    // Create user database
    addUser(&db, "admin", "admin123", true);
    addUser(&db, "john", "john123", false);
    addUser(&db, "jane", "jane123", false);

    Session session;
    initSession(&session);

    // Test Case 1: Not logged in
    printf("Test Case 1 - Not logged in:\\n");
    printf("%s\\n\\n", redirectUser(&session));

    // Test Case 2: Logged in as regular user (john)
    printf("Test Case 2 - Logged in as regular user (john):\\n");
    if (authenticate("john", "john123", &db)) {
        login(&session, findUser(&db, "john"));
    }
    printf("%s\\n", redirectUser(&session));
    logout(&session);
    printf("\\n");

    // Test Case 3: Logged in as admin
    printf("Test Case 3 - Logged in as admin:\\n");
    if (authenticate("admin", "admin123", &db)) {
        login(&session, findUser(&db, "admin"));
    }
    printf("%s\\n", redirectUser(&session));
    logout(&session);
    printf("\\n");

    // Test Case 4: Failed login attempt
    printf("Test Case 4 - Failed login attempt:\\n");
    if (authenticate("john", "wrongpassword", &db)) {
        login(&session, findUser(&db, "john"));
    }
    printf("%s\\n\\n", redirectUser(&session));

    // Test Case 5: Logged in as another regular user (jane)
    printf("Test Case 5 - Logged in as regular user (jane):\\n");
    if (authenticate("jane", "jane123", &db)) {
        login(&session, findUser(&db, "jane"));
    }
    printf("%s\\n\\n", redirectUser(&session));

    return 0;
}
