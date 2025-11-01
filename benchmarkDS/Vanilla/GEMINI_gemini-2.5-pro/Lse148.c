#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// A simple struct to hold message data
typedef struct {
    char* username;
    char* message;
} Message;

// Simulating a database with a dynamically allocated array of Message structs
Message* messagesDb = NULL;
size_t db_size = 0;
size_t db_capacity = 0;

/**
 * Cleans up all dynamically allocated memory.
 */
void cleanup_db() {
    for (size_t i = 0; i < db_size; ++i) {
        free(messagesDb[i].username);
        free(messagesDb[i].message);
    }
    free(messagesDb);
    messagesDb = NULL;
    db_size = 0;
    db_capacity = 0;
}

/**
 * Inserts a user's message into our simulated database.
 * @param username The name of the user.
 * @param message The content of the message.
 */
void insert_user_message_in_db(const char* username, const char* message) {
    // Check if we need to resize the database
    if (db_size >= db_capacity) {
        db_capacity = (db_capacity == 0) ? 4 : db_capacity * 2;
        Message* newDb = realloc(messagesDb, db_capacity * sizeof(Message));
        if (newDb == NULL) {
            fprintf(stderr, "Failed to allocate memory for DB.\n");
            return;
        }
        messagesDb = newDb;
    }

    // Allocate memory and copy the strings
    messagesDb[db_size].username = malloc(strlen(username) + 1);
    messagesDb[db_size].message = malloc(strlen(message) + 1);

    if (messagesDb[db_size].username == NULL || messagesDb[db_size].message == NULL) {
         fprintf(stderr, "Failed to allocate memory for message strings.\n");
         // Basic cleanup for this failed entry
         free(messagesDb[db_size].username);
         free(messagesDb[db_size].message);
         return;
    }

    strcpy(messagesDb[db_size].username, username);
    strcpy(messagesDb[db_size].message, message);
    db_size++;
    
    printf("DB: Message from '%s' stored successfully.\n", username);
}

/**
 * Simulates a user submitting a message.
 * It gets the username and message, inserts it into the DB, and redirects.
 * @param username The username from the request.
 * @param message The message from the request.
 */
void post(const char* username, const char* message) {
    printf("\nPOST request received for user: %s\n", username);
    insert_user_message_in_db(username, message);
    // Simulate redirecting the user
    printf("Action: Redirecting user to the main page...\n");
}

/**
 * Displays all the messages currently stored in the database.
 */
void displayMessages() {
    printf("\n--- Main Message Board ---\n");
    if (db_size == 0) {
        printf("No messages to display.\n");
    } else {
        for (size_t i = 0; i < db_size; ++i) {
            printf("%s: %s\n", messagesDb[i].username, messagesDb[i].message);
        }
    }
    printf("--------------------------\n");
}

int main() {
    // --- 5 Test Cases ---

    // Test Case 1
    post("Alice", "Hello, world!");

    // Test Case 2
    post("Bob", "This is my first post.");

    // Test Case 3
    post("Charlie", "Does this thing work?");

    // Test Case 4
    post("Alice", "I'm posting again!");

    // Test Case 5
    post("Dave", "Hi everyone!");

    // Now, display all the messages that have been posted.
    displayMessages();

    // Clean up allocated memory before exiting
    cleanup_db();

    return 0;
}