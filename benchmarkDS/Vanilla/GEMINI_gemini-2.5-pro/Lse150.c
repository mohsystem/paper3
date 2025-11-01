#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- Mock Database ---
#define MAX_MESSAGES 50
// A simple array of strings to store messages
char* mock_db[MAX_MESSAGES];
int message_count = 0;

// --- Mock Request Object ---
// Represents an incoming request with user and message data.
typedef struct {
    const char* username;
    const char* message;
} Request;

// --- Function Prototypes ---
void insert_user_message_in_db(const char* username, const char* message);
void post(Request req);
void cleanup_db();

/**
 * Simulates inserting a user's message into a database.
 * @param username The user who sent the message.
 * @param message The content of the message.
 */
void insert_user_message_in_db(const char* username, const char* message) {
    if (message_count >= MAX_MESSAGES) {
        printf("DATABASE ERROR: Database is full.\n");
        return;
    }
    printf("DATABASE: Inserting message for user '%s'.\n", username);
    
    // Format the string "username: message"
    size_t len = strlen(username) + strlen(message) + 3; // +3 for ": \0"
    char* db_entry = (char*)malloc(len * sizeof(char));
    if (db_entry == NULL) {
        printf("MEMORY ERROR: Failed to allocate memory for DB entry.\n");
        return;
    }
    snprintf(db_entry, len, "%s: %s", username, message);
    
    mock_db[message_count] = db_entry;
    message_count++;
}

/**
 * This function simulates being called when a user submits a message from a form.
 * It gets the username and message from the request.
 * It calls the insert_user_message_in_db() function.
 * It simulates redirecting the user back to the main page.
 * @param req The mock request object containing user data.
 */
void post(Request req) {
    printf("\n--- Handling POST request ---\n");
    // 1. Get username and message from the request.
    const char* username = req.username;
    const char* message = req.message;
    printf("Received post from '%s' with message: '%s'\n", username, message);

    // 2. Insert the message into the database.
    insert_user_message_in_db(username, message);
    
    // 3. Redirect the user back to the main page.
    printf("ACTION: Redirecting user to the main page...\n");
    printf("--- Request Handled ---\n");
}

/**
 * Frees all memory allocated for the mock database.
 */
void cleanup_db() {
    for (int i = 0; i < message_count; i++) {
        free(mock_db[i]);
        mock_db[i] = NULL;
    }
}

/**
 * The main page would contain an HTML form like this:
 * 
 * <form action="/post_message" method="post">
 *   <label for="username">Username:</label><br>
 *   <input type="text" id="username" name="username"><br>
 *   <label for="message">Message:</label><br>
 *   <textarea id="message" name="message" rows="4" cols="50"></textarea><br><br>
 *   <input type="submit" value="Submit">
 * </form> 
 */
int main() {
    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1
    Request req1 = {"Alice", "Hello World!"};
    post(req1);

    // Test Case 2
    Request req2 = {"Bob", "This is a test message."};
    post(req2);

    // Test Case 3
    Request req3 = {"Charlie", "How is everyone?"};
    post(req3);

    // Test Case 4
    Request req4 = {"Alice", "I'm posting again."};
    post(req4);

    // Test Case 5
    Request req5 = {"David", "Final test case."};
    post(req5);

    printf("\n--- Final State of Mock Database ---\n");
    for (int i = 0; i < message_count; i++) {
        printf("%d. %s\n", i + 1, mock_db[i]);
    }
    
    // Clean up allocated memory
    cleanup_db();
    
    return 0;
}