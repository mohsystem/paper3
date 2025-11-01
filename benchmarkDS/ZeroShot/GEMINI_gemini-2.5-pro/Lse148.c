#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple structure to hold message information
typedef struct {
    char* username;
    char* content;
} Message;

// A structure to manage our dynamic array of messages
typedef struct {
    Message** messages;
    size_t size;
    size_t capacity;
} Database;

// Global database instance
Database messageDatabase = {NULL, 0, 0};

/**
 * Sanitizes user input to prevent Cross-Site Scripting (XSS) attacks.
 * IMPORTANT: The caller is responsible for freeing the returned string.
 *
 * @param input The raw string from the user.
 * @return A newly allocated, sanitized string safe for HTML rendering, or NULL on failure.
 */
char* htmlEscape(const char* input) {
    if (!input) return NULL;

    size_t new_len = strlen(input);
    for (size_t i = 0; input[i] != '\0'; i++) {
        if (input[i] == '&') new_len += 4; // &amp;
        else if (input[i] == '<' || input[i] == '>') new_len += 3; // &lt; &gt;
        else if (input[i] == '"') new_len += 5; // &quot;
        else if (input[i] == '\'') new_len += 4; // &#39;
    }

    char* escaped = (char*)malloc(new_len + 1);
    if (!escaped) return NULL;

    char* p = escaped;
    for (size_t i = 0; input[i] != '\0'; i++) {
        if (input[i] == '&') p += sprintf(p, "&amp;");
        else if (input[i] == '<') p += sprintf(p, "&lt;");
        else if (input[i] == '>') p += sprintf(p, "&gt;");
        else if (input[i] == '"') p += sprintf(p, "&quot;");
        else if (input[i] == '\'') p += sprintf(p, "&#39;");
        else *p++ = input[i];
    }
    *p = '\0';
    return escaped;
}

/**
 * Inserts a user's message into our mock database.
 *
 * @param username The name of the user posting the message.
 * @param message  The content of the message.
 */
void insert_user_message_in_db(const char* username, const char* message) {
    if (!username || !message || strlen(username) == 0 || strlen(message) == 0) {
        printf("Error: Username and message cannot be empty.\n");
        return;
    }
    if (strlen(username) > 50 || strlen(message) > 280) {
        printf("Error: Username or message is too long.\n");
        return;
    }

    if (messageDatabase.size >= messageDatabase.capacity) {
        size_t new_capacity = messageDatabase.capacity == 0 ? 8 : messageDatabase.capacity * 2;
        Message** new_messages = (Message**)realloc(messageDatabase.messages, new_capacity * sizeof(Message*));
        if (!new_messages) {
            perror("Failed to expand database");
            return;
        }
        messageDatabase.messages = new_messages;
        messageDatabase.capacity = new_capacity;
    }

    Message* new_msg = (Message*)malloc(sizeof(Message));
    if (!new_msg) {
        perror("Failed to allocate message");
        return;
    }
    
    new_msg->username = strdup(username);
    new_msg->content = strdup(message);

    if (!new_msg->username || !new_msg->content) {
        perror("Failed to duplicate strings");
        free(new_msg->username);
        free(new_msg->content);
        free(new_msg);
        return;
    }

    messageDatabase.messages[messageDatabase.size++] = new_msg;
    printf("Message from '%s' stored successfully.\n", username);
}

/**
 * Simulates a user submitting a post.
 *
 * @param username The name of the user from the request.
 * @param message  The message content from the request.
 */
void post(const char* username, const char* message) {
    printf("\n--- Received new post from user: %s ---\n", username);
    insert_user_message_in_db(username, message);
    printf("...Redirecting user to the main page.\n");
}

/**
 * Displays all messages from the mock database in a secure manner.
 */
void display_all_messages() {
    printf("\n--- Main Page: All Messages ---\n");
    if (messageDatabase.size == 0) {
        printf("No messages to display.\n");
    } else {
        for (size_t i = 0; i < messageDatabase.size; i++) {
            Message* msg = messageDatabase.messages[i];
            
            // Sanitize output just before displaying it to prevent stored XSS.
            char* safeUsername = htmlEscape(msg->username);
            char* safeContent = htmlEscape(msg->content);

            if (safeUsername && safeContent) {
                printf("User: %s\n", safeUsername);
                printf("Message: %s\n", safeContent);
                printf("---------------------------------\n");
            }

            // Free the memory allocated by htmlEscape
            free(safeUsername);
            free(safeContent);
        }
    }
}

/**
 * Frees all dynamically allocated memory for the database.
 */
void cleanup_database() {
    for (size_t i = 0; i < messageDatabase.size; i++) {
        free(messageDatabase.messages[i]->username);
        free(messageDatabase.messages[i]->content);
        free(messageDatabase.messages[i]);
    }
    free(messageDatabase.messages);
}

int main() {
    // --- Test Cases ---

    // Test Case 1: Normal post
    post("Alice", "Hello, world! This is my first message.");

    // Test Case 2: Another normal post
    post("Bob", "This is a great platform!");

    // Test Case 3: Post with special HTML characters
    post("Charlie", "My thoughts on C++: it's great for when you need <performance> & control.");

    // Test Case 4: A malicious post attempting an XSS attack
    post("Eve", "<script>alert('You have been hacked!');</script>");

    // Test Case 5: Post with quotes
    post("David", "He said, \"It's a 'nice' day.\"");

    // Display all the messages at the end
    display_all_messages();

    // Clean up all allocated memory
    cleanup_database();

    return 0;
}