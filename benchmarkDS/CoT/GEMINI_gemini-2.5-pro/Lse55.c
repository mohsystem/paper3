#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Note: C does not have built-in web server capabilities.
// This code simulates the logic of a web application's request handlers.

// --- Data Structures for Simulation ---

#define MAX_USERS 10
#define MAX_SESSIONS 10
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_ROLE_LEN 20

typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char role[MAX_ROLE_LEN];
} User;

typedef struct {
    int sessionId;
    char username[MAX_USERNAME_LEN];
} Session;

// --- In-memory "Databases" ---
User userDatabase[MAX_USERS];
int userCount = 0;

Session sessionDatabase[MAX_SESSIONS];
int sessionCount = 0;
int nextSessionId = 1000;

// Helper function to find a user by username
User* find_user(const char* username) {
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            return &userDatabase[i];
        }
    }
    return NULL;
}

// Function to initialize the simulated user database
void initialize_database() {
    // NOTE: Storing plain text passwords is a major security vulnerability.
    // In a real C application, use a library like libsodium or libbcrypt
    // to hash passwords securely.
    strcpy(userDatabase[0].username, "user1");
    strcpy(userDatabase[0].password, "pass123");
    strcpy(userDatabase[0].role, "user");
    userCount++;

    strcpy(userDatabase[1].username, "admin");
    strcpy(userDatabase[1].password, "admin123");
    strcpy(userDatabase[1].role, "admin");
    userCount++;
}

// --- Simulated Web Request Handlers ---

/**
 * Function 1: login (GET request)
 * Simulates a GET request to the login page.
 * Returns a string representing the HTML content.
 */
const char* login_page() {
    printf("GET /login -> Displaying login form.\n");
    return "<html><body><form>...</form></body></html>";
}

/**
 * Function 2: do_login (POST request)
 * Simulates processing login credentials.
 * Returns a session ID on success, -1 on failure.
 */
int do_login(const char* username, const char* password) {
    printf("POST /do_login -> Attempting to log in user: %s\n", username);

    // Basic input validation
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        printf("Login failed: Username or password is empty.\n");
        return -1;
    }

    User* user = find_user(username);

    // In a real application, compare a hash of the provided password.
    if (user != NULL && strcmp(user->password, password) == 0) {
        if (sessionCount < MAX_SESSIONS) {
            int sessionId = nextSessionId++;
            sessionDatabase[sessionCount].sessionId = sessionId;
            strcpy(sessionDatabase[sessionCount].username, username);
            sessionCount++;
            printf("Login successful for user '%s'. Session ID: %d\n", username, sessionId);
            return sessionId;
        } else {
            printf("Login failed: Maximum sessions reached.\n");
            return -1;
        }
    } else {
        printf("Login failed: Invalid credentials for user '%s'.\n", username);
        return -1;
    }
}

/**
 * Function 3: index (GET request)
 * Simulates a GET request to the public index page.
 */
const char* index_page() {
    printf("GET / -> Displaying public index page.\n");
    return "Welcome to the public index page!";
}

// Helper to get username from session ID
const char* get_username_from_session(int sessionId) {
    for (int i = 0; i < sessionCount; ++i) {
        if (sessionDatabase[i].sessionId == sessionId) {
            return sessionDatabase[i].username;
        }
    }
    return NULL;
}

/**
 * Function 4: user_page (GET request)
 * Simulates a GET request to a protected user page.
 * Returns a dynamically allocated string with the response. The caller must free it.
 */
char* user_page(int sessionId) {
    printf("GET /user -> Access attempt with session: %d\n", sessionId);
    const char* username = get_username_from_session(sessionId);
    char* response = (char*)malloc(256 * sizeof(char));
    if (response == NULL) return NULL;

    if (username != NULL) {
        sprintf(response, "Welcome to the user page, %s!", username);
    } else {
        strcpy(response, "Unauthorized: Please log in to see this page.");
    }
    return response;
}

/**
 * Function 5: admin_page (GET request)
 * Simulates a GET request to a protected admin page.
 * Returns a dynamically allocated string with the response. The caller must free it.
 */
char* admin_page(int sessionId) {
    printf("GET /admin -> Access attempt with session: %d\n", sessionId);
    const char* username = get_username_from_session(sessionId);
    char* response = (char*)malloc(256 * sizeof(char));
    if (response == NULL) return NULL;

    if (username != NULL) {
        User* user = find_user(username);
        if (user != NULL && strcmp(user->role, "admin") == 0) {
            sprintf(response, "Welcome to the ADMIN page, %s!", username);
        } else {
            strcpy(response, "Forbidden: You do not have admin privileges to see this page.");
        }
    } else {
        strcpy(response, "Forbidden: You do not have admin privileges to see this page.");
    }
    return response;
}

// Lse55: Although C doesn't have classes, the main function serves as the test runner.
int main() {
    initialize_database();
    char* response;

    printf("--- Test Case 1: Accessing a protected page without logging in ---\n");
    response = user_page(-1); // Use -1 for invalid session
    printf("Response: %s\n\n", response);
    free(response);

    printf("--- Test Case 2: A failed login attempt with wrong password ---\n");
    int failedSessionId = do_login("user1", "wrongpassword");
    printf("Session ID returned: %d\n", failedSessionId);
    response = user_page(failedSessionId);
    printf("Accessing user page with failed session: %s\n\n", response);
    free(response);

    printf("--- Test Case 3: A successful user login and accessing the user page ---\n");
    int userSessionId = do_login("user1", "pass123");
    printf("Session ID returned: %d\n", userSessionId);
    response = user_page(userSessionId);
    printf("Response: %s\n\n", response);
    free(response);

    printf("--- Test Case 4: A logged-in user trying to access the admin page (should fail) ---\n");
    response = admin_page(userSessionId);
    printf("Response: %s\n\n", response);
    free(response);

    printf("--- Test Case 5: A successful admin login and accessing the admin page ---\n");
    int adminSessionId = do_login("admin", "admin123");
    printf("Session ID returned: %d\n", adminSessionId);
    response = user_page(adminSessionId);
    printf("Accessing user page: %s\n", response);
    free(response);
    response = admin_page(adminSessionId);
    printf("Accessing admin page: %s\n\n", response);
    free(response);

    return 0;
}