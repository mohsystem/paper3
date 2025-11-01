#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_USERNAME_LENGTH 50
#define MAX_MESSAGE_LENGTH 280

// --- Data Structures for simulated DB ---
typedef struct MessageNode {
    char* message;
    struct MessageNode* next;
} MessageNode;

typedef struct UserNode {
    char* username;
    MessageNode* messages;
    struct UserNode* next;
} UserNode;

// Head of the user list (our simulated DB)
static UserNode* g_user_db = NULL;

// --- Function Declarations ---
void insert_user_message_in_db(const char* username, const char* message);
const char* post(const char* username, const char* message);
void cleanup_db();
int is_string_blank(const char *str);

/**
 * Inserts a user's message into the simulated database.
 * Handles memory allocation and linked list logic.
 */
void insert_user_message_in_db(const char* username, const char* message) {
    printf("DB: Inserting message for user '%s'.\n", username);
    // SECURITY: In a real application, use parameterized queries to prevent SQL injection.
    
    UserNode* currentUser = g_user_db;
    UserNode* lastUser = NULL;

    while (currentUser != NULL && strcmp(currentUser->username, username) != 0) {
        lastUser = currentUser;
        currentUser = currentUser->next;
    }
    
    if (currentUser == NULL) {
        currentUser = (UserNode*)malloc(sizeof(UserNode));
        if (!currentUser) { fprintf(stderr, "Error: Memory allocation failed.\n"); return; }
        
        currentUser->username = (char*)malloc(strlen(username) + 1);
        if (!currentUser->username) { fprintf(stderr, "Error: Memory allocation failed.\n"); free(currentUser); return; }
        strcpy(currentUser->username, username);
        
        currentUser->messages = NULL;
        currentUser->next = NULL;

        if (g_user_db == NULL) g_user_db = currentUser;
        else lastUser->next = currentUser;
    }

    MessageNode* newMessage = (MessageNode*)malloc(sizeof(MessageNode));
    if (!newMessage) { fprintf(stderr, "Error: Memory allocation failed.\n"); return; }
    
    newMessage->message = (char*)malloc(strlen(message) + 1);
    if (!newMessage->message) { fprintf(stderr, "Error: Memory allocation failed.\n"); free(newMessage); return; }
    strcpy(newMessage->message, message);

    newMessage->next = currentUser->messages;
    currentUser->messages = newMessage;
}

/**
 * Checks if a string is null, empty, or contains only whitespace.
 */
int is_string_blank(const char *str) {
    if (str == NULL) return 1;
    for (; *str; str++) {
        if (!isspace((unsigned char)*str)) return 0;
    }
    return 1;
}

/**
 * Simulates handling a POST request to submit a message.
 */
const char* post(const char* username, const char* message) {
    // SECURITY: Validate all user input before processing.
    // 1. Check for null or effectively empty strings.
    if (is_string_blank(username)) {
        fprintf(stderr, "Error: Username cannot be empty.\n");
        return "redirect:/main?error=invalid_username";
    }
    if (is_string_blank(message)) {
        fprintf(stderr, "Error: Message cannot be empty.\n");
        return "redirect:/main?error=invalid_message";
    }

    // 2. Check for excessive length to prevent buffer overflows in subsequent copies.
    if (strlen(username) > MAX_USERNAME_LENGTH) {
        fprintf(stderr, "Error: Username is too long.\n");
        return "redirect:/main?error=username_too_long";
    }
    if (strlen(message) > MAX_MESSAGE_LENGTH) {
        fprintf(stderr, "Error: Message is too long.\n");
        return "redirect:/main?error=message_too_long";
    }
    
    insert_user_message_in_db(username, message);
    
    printf("Success! Redirecting to the main page.\n");
    return "redirect:/main";
}

/**
 * Frees all dynamically allocated memory for the database.
 */
void cleanup_db() {
    UserNode* currentUser = g_user_db;
    while (currentUser != NULL) {
        MessageNode* currentMsg = currentUser->messages;
        while (currentMsg != NULL) {
            MessageNode* nextMsg = currentMsg->next;
            free(currentMsg->message);
            free(currentMsg);
            currentMsg = nextMsg;
        }
        UserNode* nextUser = currentUser->next;
        free(currentUser->username);
        free(currentUser);
        currentUser = nextUser;
    }
    g_user_db = NULL;
}

void print_db_state() {
    printf("\n--- Final Database State ---\n");
    for (UserNode* u = g_user_db; u != NULL; u = u->next) {
        printf("User: %s\n", u->username);
        for (MessageNode* m = u->messages; m != NULL; m = m->next) {
            printf("  - %s\n", m->message);
        }
    }
}

// In C, there is no main class. We use a main function as the entry point.
// We can name it "Lse150" in a comment if needed.
int main() {
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Valid submission
    printf("\n--- Test Case 1: Valid submission ---\n");
    post("alice", "Hello, world!");

    // Test Case 2: Another valid submission
    printf("\n--- Test Case 2: Another valid user ---\n");
    post("bob", "This is a test message.");

    // Test Case 3: Message is too long
    printf("\n--- Test Case 3: Message too long ---\n");
    char longMessage[301];
    memset(longMessage, 'a', 300);
    longMessage[300] = '\0';
    post("alice", longMessage);

    // Test Case 4: Empty username
    printf("\n--- Test Case 4: Empty username ---\n");
    post("   ", "This should fail.");
    
    // Test Case 5: Potentially malicious input
    printf("\n--- Test Case 5: Input with special characters ---\n");
    post("charlie", "<script>alert('xss')</script>");

    print_db_state();

    // SECURITY: Always free allocated memory to prevent memory leaks.
    cleanup_db();
    printf("\nDatabase cleaned up.\n");
    
    return 0;
}