#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_USERNAME_LEN 50
#define MAX_MESSAGE_LEN 256
#define MAX_MESSAGES 100

// Structure to hold a single message
typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    char message[MAX_MESSAGE_LEN + 1];
} Message;

// Simulating a database with a static array of Message structs
Message messageDatabase[MAX_MESSAGES];
int messageCount = 0;

/**
 * Sanitizes a string by copying it to an output buffer and replacing
 * special HTML characters. Guards against buffer overflows.
 * @param input The source string to sanitize.
 * @param output The destination buffer.
 * @param output_size The size of the destination buffer.
 */
void sanitize(const char* input, char* output, size_t output_size) {
    if (!input || !output || output_size == 0) return;
    
    size_t out_idx = 0;
    for (size_t i = 0; input[i] != '\0' && out_idx < output_size - 1; ++i) {
        const char* replacement = NULL;
        size_t repl_len = 0;

        switch (input[i]) {
            case '&': replacement = "&amp;"; repl_len = 5; break;
            case '<': replacement = "&lt;"; repl_len = 4; break;
            case '>': replacement = "&gt;"; repl_len = 4; break;
            case '"': replacement = "&quot;"; repl_len = 6; break;
            case '\'': replacement = "&#x27;"; repl_len = 6; break;
        }

        if (replacement) {
            if (out_idx + repl_len < output_size) {
                strcpy(output + out_idx, replacement);
                out_idx += repl_len;
            } else {
                break; // Not enough space for replacement
            }
        } else {
            output[out_idx++] = input[i];
        }
    }
    output[out_idx] = '\0'; // Ensure null termination
}

/**
 * Inserts a user's message into the simulated database.
 * @param username The sanitized username.
 * @param message The sanitized message.
 */
void insert_user_message_in_db(const char* username, const char* message) {
    if (messageCount >= MAX_MESSAGES) {
        printf("ERROR: Database is full. Cannot store message.\n");
        return;
    }
    strncpy(messageDatabase[messageCount].username, username, MAX_USERNAME_LEN);
    messageDatabase[messageCount].username[MAX_USERNAME_LEN] = '\0';

    strncpy(messageDatabase[messageCount].message, message, MAX_MESSAGE_LEN);
    messageDatabase[messageCount].message[MAX_MESSAGE_LEN] = '\0';
    
    printf("LOG: Message from '%s' stored in DB.\n", username);
    messageCount++;
}

bool is_string_empty(const char* str) {
    if (!str) return true;
    while (*str) {
        if (*str != ' ' && *str != '\t' && *str != '\n' && *str != '\r') {
            return false;
        }
        str++;
    }
    return true;
}

/**
 * Simulates handling a POST request to submit a message.
 * @param username The raw username from the request.
 * @param message The raw message from the request.
 */
void post(const char* username, const char* message) {
    // 1. Input validation
    if (is_string_empty(username) || is_string_empty(message)) {
        printf("ERROR: Username or message cannot be empty.\n");
        return;
    }
    if (strlen(username) > MAX_USERNAME_LEN) {
        printf("ERROR: Username is too long.\n");
        return;
    }
    if (strlen(message) > MAX_MESSAGE_LEN) {
        printf("ERROR: Message is too long.\n");
        return;
    }

    // 2. Sanitize inputs to prevent XSS. Use temporary buffers.
    // Allocate enough space for worst-case expansion (e.g., every char is '&')
    char sanitizedUsername[MAX_USERNAME_LEN * 5 + 1];
    char sanitizedMessage[MAX_MESSAGE_LEN * 6 + 1];
    
    sanitize(username, sanitizedUsername, sizeof(sanitizedUsername));
    sanitize(message, sanitizedMessage, sizeof(sanitizedMessage));

    // 3. Insert into database
    insert_user_message_in_db(sanitizedUsername, sanitizedMessage);

    // 4. Redirect
    printf("Redirecting user to the main page...\n\n");
}

/**
 * Displays all the messages currently stored in the database.
 */
void displayMessages() {
    printf("--- Displaying All Messages ---\n");
    if (messageCount == 0) {
        printf("No messages to display.\n");
    } else {
        for (int i = 0; i < messageCount; i++) {
            printf("%s: %s\n", messageDatabase[i].username, messageDatabase[i].message);
        }
    }
    printf("-----------------------------\n\n");
}

int main() {
    printf("Initial State:\n");
    displayMessages();

    // --- Test Cases ---
    printf(">>> Running Test Cases <<<\n");

    // Test Case 1: Standard post
    post("Alice", "Hello World!");

    // Test Case 2: Another standard post
    post("Bob", "This is a test message.");

    // Test Case 3: Post with characters that need sanitization
    post("Charlie", "<script>alert('xss')</script>");

    // Test Case 4: Post with an empty message (should be rejected)
    post("David", "  ");

    // Test Case 5: Post from a user who posted before
    post("Alice", "I am posting again!");

    printf(">>> Test Cases Finished <<<\n\n");

    printf("Final State:\n");
    displayMessages();

    return 0;
}